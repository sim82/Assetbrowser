#include "MyGLWidget.h"
#include "SceneMeshDBViewer.h"
#include "qopenglvertexarrayobject.h"
#include "scene.capnp.h"
#include <capnp/serialize.h>

#include <QFile>

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QTimer>
#include <iostream>

MyGLWidget::MyGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , moveTimer(new QTimer(this))
{
    moveTimer->setObjectName("moveTimer");
    QMetaObject::connectSlotsByName(this);

    moveTimer->setInterval(1000 / 60);
    moveTimer->setSingleShot(false);

    setFocusPolicy(Qt::ClickFocus);

#if 1
    QSurfaceFormat f = QSurfaceFormat::defaultFormat();
    f.setDepthBufferSize(24);
    setFormat(f);
    std::cout << "format " << f.redBufferSize() << " " << f.depthBufferSize() << std::endl;
#endif
    // setFormat();

    //    format.setDepthBufferSize(16);
    //    setFormat(format);
    //    dbFile.open(QFile::ReadOnly);

    //    size = dbFile.size();
    //    ptr = dbFile.map(0, size);
}

MyGLWidget::~MyGLWidget() {}

void MyGLWidget::setNavigatable(std::unique_ptr<GLNavigatable> navigatable)
{
    navigatable_.reset( navigatable.release() ); // TODO: awkward...
}

void MyGLWidget::initializeGL()
{
    QOpenGLWidget::initializeGL();
    QOpenGLFunctions::initializeOpenGLFunctions();

    QSurfaceFormat f = format();
    std::cout << "format " << f.redBufferSize() << " " << f.depthBufferSize() << std::endl;



    //    vao.bind();
    //    glVertexPointer();t
}

void MyGLWidget::resizeGL(int w, int h) { QOpenGLWidget::resize(w, h); }

void MyGLWidget::paintGL()
{
    QOpenGLWidget::paintGL();
    makeCurrent();

    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 mat;

    mat.perspective(60, width() / float(height()), .1, 100);
    mat.rotate(lat_, QVector3D(1, 0, 0));
    mat.rotate(lon_, QVector3D(0, 1, 0));
    mat.translate(QVector3D() - pos_);
    //glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mat.data());

    //    glEnable(GL_LIGHTING);
    //    glEnable(GL_LIGHT0);

    ////    GLfloat vpos[3] = {pos_.x(), pos_.y(), pos_.z()};
    ////    glLightfv(GL_LIGHT0, GL_POSITION, vpos);
    ////    GLfloat cyan[] = {0.f, .8f, .8f, 1.f};
    ////    glMaterialfv(GL_FRONT, GL_DIFFUSE, cyan);

//    glDisable(GL_CULL_FACE);
//    glColor3f(1.0, 0.0, 0.0);
//    glBegin(GL_TRIANGLES);
//    glVertex3f(0, 0, 0);
//    glVertex3f(1, 0, 0);
//    glVertex3f(1, 1, 0);
//    glEnd();


    if (navigatable_)
    {
        navigatable_->draw();
    }

    doneCurrent();

    //    capnp::ReaderOptions ro;
    //    ro.traversalLimitInWords = 1024 * 1024 * 1024;
    //    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<capnp::word>((capnp::word *) ptr, size /
    //    sizeof(capnp::word)), ro);

    //    cp::scene::ShaderMeshDB::Reader dbReader = reader.getRoot<cp::scene::ShaderMeshDB>();

    //    for( int i = 0; i < dbReader.getMeshes().size(); ++i )
    //    {
    //        cp::scene::ShaderMesh::Reader shadermeshReader = dbReader.getMeshes()[i];

    //        if( !shadermeshReader.hasLattice() )
    //        {
    //            continue;
    //        }

    //        capnp::List<cp::scene::ShaderMesh::Lattice::Mesh>::Reader meshlistReader =
    //        shadermeshReader.getLattice().getMeshes();
    //        cp::scene::AttributeArrayInterleaved::Reader arrayReader = meshlistReader[0].getArray();
    //        capnp::List<cp::scene::AttributeArrayInterleaved::Attribute>::Reader attributeList =
    //        arrayReader.getAttributes();

    //        bool breakOuter = false;
    //        for( int j = 0; j < attributeList.size(); ++j )
    //        {
    //            cp::scene::AttributeArrayInterleaved::Attribute::Reader attribute = attributeList[j];
    //            if( attribute.getName() == "pos" )
    //            {
    //                uint32_t stride = arrayReader.getAttributeStride();
    //                uint32_t offset = attribute.getOffset();
    //                uint32_t numIndex = arrayReader.getNumIndex();

    //                QOpenGLBuffer vertexBuffer;
    //                QOpenGLBuffer indexBuffer;
    //                {
    //                    auto it = elementArrays_.find(i);

    //                    if( it == elementArrays_.end() )
    //                    {
    //                        QOpenGLBuffer buffer(QOpenGLBuffer::VertexBuffer);
    //                        buffer.create();
    //                        buffer.bind();

    //                        capnp::Data::Reader attributearrayReader = arrayReader.getAttributeArray();
    //                        buffer.allocate(attributearrayReader.begin(), attributearrayReader.size());

    //                        elementArrays_[i] = buffer;
    //                        vertexBuffer = buffer;
    //                    }
    //                    else
    //                    {
    //                        vertexBuffer = it.value();
    //                        vertexBuffer.bind();
    //                    }
    //                }

    //                {
    //                    auto it = indexArrays_.find(i);

    //                    if( it == indexArrays_.end() )
    //                    {
    //                        QOpenGLBuffer buffer(QOpenGLBuffer::IndexBuffer);
    //                        buffer.create();
    //                        buffer.bind();

    //                        capnp::Data::Reader indexarrayReader = arrayReader.getIndexArray();
    //                        buffer.allocate(indexarrayReader.begin(), indexarrayReader.size());
    //                        indexArrays_[i] = buffer;
    //                    }
    //                    else
    //                    {
    //                        indexBuffer = it.value();
    //                        indexBuffer.bind();

    //                    }

    //                }

    //                {
    //                    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    //                    glEnable(GL_CULL_FACE);
    //                    glCullFace(GL_BACK);

    //                    glEnableVertexAttribArray(0);
    //                    glVertexAttribPointer(
    //                       0,                  // attribute 0. No particular reason for 0, but must match the layout
    //                       in the shader.
    //                       3,                  // size
    //                       GL_FLOAT,           // type
    //                       GL_FALSE,           // normalized?
    //                       stride,                  // stride
    //                       (void*)offset            // array buffer offset
    //                    );

    //                    glDrawElements(GL_TRIANGLES, numIndex, GL_UNSIGNED_INT, (GLvoid*)nullptr);
    //                    glDisableVertexAttribArray(0);

    //                }

    //                breakOuter = true;
    //                break;
    //            }
    //        }
    //        if( breakOuter )
    //        {
    //            //break;
    //        }
    //    }
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        mousePressed_ = true;
        event->accept();
        lastMousePos_ = event->screenPos();
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        mousePressed_ = false;
        event->accept();
    }
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mousePressed_)
    {
        QPointF delta = event->screenPos() - lastMousePos_;
        lastMousePos_ = event->screenPos();
        lon_ += delta.x();
        lat_ += delta.y();

        while (lon_ >= 360.0)
        {
            lon_ -= 360.0;
        }
        while (lon_ < 0)
        {
            lon_ += 360.0;
        }

        if (lat_ > 90.0)
        {
            lat_ = 90.0;
        }
        else if (lat_ < -90)
        {
            lat_ = -90.0;
        }

        // std::cout << lon_ << " " << lat_ << std::endl;

        stateChanged_ = true;

        if (!moveTimer->isActive())
        {
            moveTimer->start();
        }
    }
}

void MyGLWidget::keyPressEvent(QKeyEvent *event)
{
    getKey(Qt::Key(event->key())) = true;

    event->accept();

    if (anyKeyPressed())
    {
        stateChanged_ = true;

        if (!moveTimer->isActive())
        {
            moveTimer->start();
        }
    }
}

void MyGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    getKey(Qt::Key(event->key())) = false;

    event->accept();

    //    if( !anyKeyPressed() )
    //    {
    //        forwardVelocity_ = QVector3D();
    //        rightVelocity_ = QVector3D();
    //        moveTimer->stop();
    //    }
}

void MyGLWidget::updateMove()
{

    if (!stateChanged_ && !anyKeyPressed())
    {
        moveTimer->stop();
    }

    forwardVelocity_ = QVector3D();
    rightVelocity_   = QVector3D();

    stateChanged_ = false;
    std::cout << "update move" << std::endl;
    QMatrix4x4 matrix;

    matrix.rotate(-lon_, QVector3D(0, 1, 0));
    matrix.rotate(-lat_, QVector3D(1, 0, 0));

    QVector3D right   = matrix * QVector3D(1, 0, 0);
    QVector3D forward = matrix * QVector3D(0, 0, -1);

    if (forwardPressed_)
    {
        forwardVelocity_ += forward;
    }

    if (backwardPressed_)
    {
        forwardVelocity_ -= forward;
    }
    if (leftPressed_)
    {
        rightVelocity_ -= right;
    }
    if (rightPressed_)
    {
        rightVelocity_ += right;
    }

    forwardVelocity_.normalize();
    rightVelocity_.normalize();

    pos_ += (forwardVelocity_ + rightVelocity_);

    std::cout << "pos " << pos_.x() << " " << pos_.y() << " " << pos_.z() << " " << lon_ << " " << lat_ << std::endl;
}

void MyGLWidget::on_moveTimer_timeout()
{
    updateMove();
    update();
}

bool &MyGLWidget::getKey(const Qt::Key &key)
{
    switch (key)
    {
    case Qt::Key_W:
        return forwardPressed_;
    case Qt::Key_S:
        return backwardPressed_;
    case Qt::Key_A:
        return leftPressed_;
    case Qt::Key_D:
        return rightPressed_;

    default:
        return nullPressed_;
    }
}

bool MyGLWidget::anyKeyPressed() const { return forwardPressed_ || backwardPressed_ || leftPressed_ || rightPressed_; }
