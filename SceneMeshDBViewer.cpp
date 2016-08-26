#include "SceneMeshDBViewer.h"
#include "capnp/serialize.h"
#include "scene.capnp.h"
#include <QVector3D>
#include <iostream>
#include <memory>

SceneMeshDBViewer::SceneMeshDBViewer(cp::asset::AssetMeshData::Reader reader)
    : messageBuilder_(std::make_unique<capnp::MallocMessageBuilder>())
{
    auto builder = messageBuilder_->initRoot<cp::asset::AssetMeshData>();
    builder.setAppearanceList(reader.getAppearanceList());
    builder.setAttributeArrayInterleavedList(reader.getAttributeArrayInterleavedList());
    reader_ = builder;
}

SceneMeshDBViewer::~SceneMeshDBViewer() {}

void SceneMeshDBViewer::draw()
{
    if (!bGlFuncInit_)
    {
        QOpenGLFunctions::initializeOpenGLFunctions();
        bGlFuncInit_ = true;
    }

    glFrontFace(GL_CW);

    int i = 0;
    {

        capnp::List<::cp::scene::AttributeArrayInterleaved>::Reader meshlistReader =
            reader_.getAttributeArrayInterleavedList();

        glEnableClientState(GL_INDEX_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        for (int k = 0; k < meshlistReader.size(); ++k)
        {
            cp::scene::AttributeArrayInterleaved::Reader arrayReader = meshlistReader[k];
            capnp::List<cp::scene::AttributeArrayInterleaved::Attribute>::Reader attributeList =
                arrayReader.getAttributes();

            bool breakOuter = false;
            for (int j = 0; j < attributeList.size(); ++j)
            {
                cp::scene::AttributeArrayInterleaved::Attribute::Reader attribute = attributeList[j];
                if (attribute.getName() == "a_position")
                {
#if 1
                    glCullFace(GL_BACK);
                    //                    glEnable(GL_CULL_FACE);
                    glDisable(GL_CULL_FACE);

                    {
                        qsrand(k);
                        glColor3f(0.2 + 0.2 * (qrand() / float(RAND_MAX)), qrand() / float(RAND_MAX),
                                  0.2 + 0.2 * (qrand() / float(RAND_MAX)));

                        //                        glColor3f(0, 0, 0);
                        glLineWidth(1.0);
                        //                        glColor3f( 0.2, 1.0, 0.2 );
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                        // glDrawElements(GL_TRIANGLES, numIndex, GL_UNSIGNED_INT, (GLvoid *)nullptr);
                        capnp::Data::Reader indexarrayReader = arrayReader.getIndexArray();
                        // glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glDisable(GL_POLYGON_OFFSET_FILL);
                        drawElementsFallback(arrayReader, j);
                    }

                    // if (/* DISABLES CODE */ (false))
                    {

                        glColor3f(0, 0, 0);
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        glEnable(GL_POLYGON_OFFSET_FILL);
                        glPolygonOffset(1.0, 1.0);
                        drawElementsFallback(arrayReader, j);
                    }
#else
                    uint32_t stride   = arrayReader.getAttributeStride();
                    uint32_t offset   = attribute.getOffset();
                    uint32_t numIndex = arrayReader.getNumIndex();

                    int meshkey = i + k * 1000000;

                    QOpenGLBuffer vertexBuffer;
                    QOpenGLBuffer indexBuffer;
                    {

                        auto it = elementArrays_.find(meshkey);

                        if (it == elementArrays_.end())
                        {
                            QOpenGLBuffer buffer(QOpenGLBuffer::VertexBuffer);
                            buffer.create();
                            buffer.bind();

                            capnp::Data::Reader attributearrayReader = arrayReader.getAttributeArray();
                            // auto buf = (float const*)attributearrayReader.begin();
                            buffer.allocate(attributearrayReader.begin(), attributearrayReader.size());

                            elementArrays_[meshkey] = buffer;
                            vertexBuffer            = buffer;
                        }
                        else
                        {
                            vertexBuffer = it.value();
                            vertexBuffer.bind();
                        }
                    }

                    {
                        auto it = indexArrays_.find(meshkey);

                        if (it == indexArrays_.end())
                        {
                            QOpenGLBuffer buffer(QOpenGLBuffer::IndexBuffer);
                            buffer.create();
                            buffer.bind();

                            capnp::Data::Reader indexarrayReader = arrayReader.getIndexArray();
                            auto array                           = (uint const *)indexarrayReader.begin();
                            auto size                            = indexarrayReader.size();
                            buffer.allocate(indexarrayReader.begin(), indexarrayReader.size());
                            indexArrays_[meshkey] = buffer;
                        }
                        else
                        {
                            indexBuffer = it.value();
                            indexBuffer.bind();
                        }
                    }

                    {
                        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                        // glPolygonMode( GL_FRONT, GL_LINE );
                        glCullFace(GL_BACK);
                        glEnable(GL_CULL_FACE);
                        // glDisable(GL_CULL_FACE);

                        glEnableVertexAttribArray(0);
                        glVertexAttribPointer(
                            0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                            3, // size
                            GL_FLOAT,      // type
                            GL_FALSE,      // normalized?
                            stride,        // stride
                            (void *)offset // array buffer offset
                            );

                        // glIndexPointer(GL_INT, 0, (GLvoid*)nullptr);
                        {
                            qsrand(meshkey);

                            glColor3f(0.2 + 0.2 * (qrand() / float(RAND_MAX)), qrand() / float(RAND_MAX),
                                      0.2 + 0.2 * (qrand() / float(RAND_MAX)));

                            // glColor3f( 0.2, 1.0, 0.2 );
                            glPolygonMode(GL_FRONT, GL_LINE);

                            // glDrawElements(GL_TRIANGLES, numIndex, GL_UNSIGNED_INT, (GLvoid *)nullptr);
                            capnp::Data::Reader indexarrayReader = arrayReader.getIndexArray();
                            // glBindBuffer(GL_ARRAY_BUFFER, 0);
                            glDrawElements(GL_TRIANGLES, numIndex, GL_UNSIGNED_INT, (GLvoid *)nullptr);
                        }

                        if (/* DISABLES CODE */ (false))
                        {
                            glEnable(GL_POLYGON_OFFSET_FILL);
                            glPolygonOffset(1.0, 1.0);
                            glColor3f(0, 0, 0);
                            glPolygonMode(GL_FRONT, GL_FILL);
                            glDrawElements(GL_TRIANGLES, numIndex, GL_UNSIGNED_INT, (GLvoid *)nullptr);
                            glDisable(GL_POLYGON_OFFSET_FILL);
                        }
                        glDisableVertexAttribArray(0);
                    }
#endif
                    breakOuter = true;
                    break;
                }
            }
            if (breakOuter)
            {
                // break;
            }
        }
    }
    if (glGetError() != GL_NO_ERROR)
    {
        std::cout << "meeeeeeeeep\n";
    }
}

void SceneMeshDBViewer::drawElementsFallback(cp::scene::AttributeArrayInterleaved::Reader array, uint attributeIndex)
{
    auto attribute  = array.getAttributes()[attributeIndex];
    auto stride     = array.getAttributeStride();
    auto offset     = attribute.getOffset();
    auto numIndex   = array.getNumIndex();
    auto indexArray = (GLint const *)array.getIndexArray().begin();

    auto base = array.getAttributeArray().begin();
#if 0
    QVector3D mid(0,0,0);
    for (int i = 0; i < numIndex; i += 3)
    {
        for (int j = 0; j < 3; ++j)
        {
            auto index = indexArray[i + j];
            auto pos   = (GLfloat const *)(base + index * stride + offset);
            mid += QVector3D(pos[0], pos[1], pos[2]);
            //glVertex3fv(pos);
        }
    }

    mid /= float(numIndex);
#endif
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numIndex; i += 3)
    {
        for (int j = 0; j < 3; ++j)
        {
            auto index = indexArray[i + j];
            auto pos   = (GLfloat const *)(base + index * stride + offset);
            glVertex3fv(pos);
            //   QVector3D xp(pos[0], pos[1], pos[2]);
            //   xp -= mid;
            //   glVertex3f(xp.x(), xp.y(), xp.z());
        }
    }
    glEnd();
}
