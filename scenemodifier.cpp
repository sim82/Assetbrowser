/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "scenemodifier.h"
#include "asset.capnp.h"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <QtCore/QDebug>
#include <QBuffer>
#include <QAttribute>
#include <QFile>
#include <memory>
#include <cassert>
#include <iostream>

class CSceneAttributeArrayInterleavedRenderer
        : public Qt3DRender::QGeometryRenderer
{

    std::vector<Qt3DRender::QAttribute*>createAttributes(cp::scene::AttributeArrayInterleaved::Reader aa, Qt3DRender::QGeometry *geometry, Qt3DRender::QBuffer *buffer )
    {
        using namespace Qt3DRender;
        std::vector<QAttribute*> attributes;

        std::map<std::string, QString> nameMap( {{"a_position", QAttribute::defaultPositionAttributeName()},
                                                {"a_texcoord", QAttribute::defaultTextureCoordinateAttributeName()},
                                                {"a_normal", QAttribute::defaultNormalAttributeName()},
                                                {"a_t", QAttribute::defaultTangentAttributeName()}});

        for( auto attribute: aa.getAttributes() )
        {
            auto it = nameMap.find(attribute.getName());
            if( it == nameMap.end() )
            {
                continue;
            }


            if( attribute.getName() == "a_position" )
            {
                auto stride     = meshData_.getAttributeStride();
                auto offset     = attribute.getOffset();
                auto numIndex   = aa.getNumIndex();
                auto indexArray = (uint16_t const *)aa.getIndexArray().begin();
                auto base = aa.getAttributeArray().asChars().begin();

                QVector3D mid(0,0,0);
                QVector3D vmin(0,0,0);
                QVector3D vmax(0,0,0);
                for (int i = 0; i < numIndex; i += 3)
                {
                    for (int j = 0; j < 3; ++j)
                    {
                        auto index = indexArray[i + j];
                        auto pos   = (float const *)(base + index * stride + offset);
                        mid += QVector3D(pos[0], pos[1], pos[2]);

                        vmin.setX(std::min(vmin.x(), pos[0]));
                        vmin.setY(std::min(vmin.y(), pos[1]));
                        vmin.setZ(std::min(vmin.z(), pos[2]));

                        vmax.setX(std::max(vmax.x(), pos[0]));
                        vmax.setY(std::max(vmax.y(), pos[1]));
                        vmax.setZ(std::max(vmax.z(), pos[2]));
                        //glVertex3fv(pos);
                    }
                }

                mid /= float(numIndex);
            }

            assert( attribute.getType() == cp::scene::AttributeArrayInterleaved::Type::FLOAT32 );

            auto qAttribute = new QAttribute(geometry);
            auto name = it->second;
            qAttribute->setName(name);
            qAttribute->setDataType(QAttribute::Float);
            qAttribute->setDataSize(attribute.getWidth());
            qAttribute->setAttributeType(QAttribute::VertexAttribute);
            qAttribute->setBuffer(buffer);
            qAttribute->setByteStride(aa.getAttributeStride());
            qAttribute->setByteOffset(attribute.getOffset());
            qAttribute->setCount(aa.getNumVertex());

            attributes.push_back(qAttribute);
        }

        return attributes;
    }

public:
#if 1
    CSceneAttributeArrayInterleavedRenderer( cp::scene::AttributeArrayInterleaved::Reader meshData, QNode *parent = nullptr)
        : Qt3DRender::QGeometryRenderer(parent)
        , messageBuilder_(std::make_unique<capnp::MallocMessageBuilder>())
    {
        messageBuilder_->setRoot(meshData);
        meshData_ = messageBuilder_->getRoot<cp::scene::AttributeArrayInterleaved>();


        using namespace Qt3DRender;

        auto geometry = new QGeometry(this);

        auto vertexArrayBuffer = new QBuffer(QBuffer::VertexBuffer, geometry);

        auto vertexArrayReader = meshData_.getAttributeArray();
        vertexArrayBuffer->setData( QByteArray::fromRawData(vertexArrayReader.asChars().begin(), vertexArrayReader.size()));
        auto qAttributes = createAttributes(meshData_, geometry, vertexArrayBuffer);


        auto indexArrayBuffer = new QBuffer(QBuffer::IndexBuffer, geometry);
        {
            auto indexArrayReader = meshData_.getIndexArray();
            indexArrayBuffer->setData( QByteArray::fromRawData(indexArrayReader.asChars().begin(), indexArrayReader.size()));
        }
        assert( meshData_.getIndexType() == cp::scene::AttributeArrayInterleaved::Type::INT16 );

        auto indexAttribute = new QAttribute(geometry);
        indexAttribute->setAttributeType(QAttribute::IndexAttribute);
        indexAttribute->setDataType(QAttribute::UnsignedShort);
        indexAttribute->setBuffer(indexArrayBuffer);

        indexAttribute->setCount(meshData_.getNumIndex());


        for( auto qa : qAttributes )
        {
            geometry->addAttribute(qa);
        }
        geometry->addAttribute(indexAttribute);
        QGeometryRenderer::setPrimitiveType(QGeometryRenderer::Triangles);
        QGeometryRenderer::setGeometry(geometry);

    }

    ~CSceneAttributeArrayInterleavedRenderer()
    {
        std::cerr << "~CSceneAttributeArrayInterleavedRenderer()" << std::endl;
    }
#else
    CSceneAttributeArrayInterleavedRenderer( cp::scene::AttributeArrayInterleaved::Reader meshData, QNode *parent = nullptr)
        : Qt3DRender::QGeometryRenderer(parent)
        , messageBuilder_(std::make_unique<capnp::MallocMessageBuilder>())
    {
        messageBuilder_->setRoot(meshData);
        meshData_ = messageBuilder_->getRoot<cp::scene::AttributeArrayInterleaved>();


        using namespace Qt3DRender;

        auto geometry = new QGeometry(this);

        auto vertexArrayBuffer = new QBuffer(QBuffer::VertexBuffer, geometry);

#if 0
        auto vertexArrayReader = meshData.getAttributeArray();
        vertexArrayBuffer->setData( QByteArray::fromRawData(vertexArrayReader.asChars().begin(), vertexArrayReader.size()));
#else
        std::vector<float>vertex( {
                                   0.0, 0.0, 0.0, /**/0.0, 0.0, /**/0.0, 1.0, 0.0, /**/1.0, 0.0, 0.0, 1.0,
                                   1.0, 0.0, 0.0, /**/1.0, 0.0, /**/0.0, 1.0, 0.0, /**/1.0, 0.0, 0.0, 1.0,
                                   1.0, 1.0, 0.0, /**/1.0, 1.0, /**/0.0, 1.0, 0.0, /**/1.0, 0.0, 0.0, 1.0
                                  });
        vertexArrayBuffer->setData(QByteArray((const char*)vertex.data(), vertex.size() * sizeof(float)));
#endif
        auto posAttribute = new QAttribute(geometry);
        auto texCoordAttribute = new QAttribute(geometry);
        auto normAttribute = new QAttribute(geometry);
        auto tanAttribute = new QAttribute(geometry);


        cp::scene::AttributeArrayInterleaved::Attribute::Reader posAttributeReader;
        for( auto attribute: meshData_.getAttributes() )
        {
            if( attribute.getName() == "a_position" )
            {
                posAttributeReader = attribute;
            }
        }
#if 0
        QVector3D mid(0,0,0);

        {
            auto stride     = meshData_.getAttributeStride();
            auto offset     = posAttributeReader.getOffset();
            auto numIndex   = meshData_.getNumIndex();
            auto indexArray = (int const *)meshData_.getIndexArray().begin();
            auto base = vertexArrayReader.asChars().begin();

            for (int i = 0; i < numIndex; i += 3)
            {
                for (int j = 0; j < 3; ++j)
                {
                    auto index = indexArray[i + j];
                    auto pos   = (float const *)(base + index * stride + offset);
                    mid += QVector3D(pos[0], pos[1], pos[2]);
                    //glVertex3fv(pos);
                }
            }

            mid /= float(numIndex);
        }
#endif
        assert( posAttributeReader.getType() == cp::scene::AttributeArrayInterleaved::Type::FLOAT32 );
        assert( posAttributeReader.getWidth() == 3 );

        posAttribute->setName(QAttribute::defaultPositionAttributeName());
        posAttribute->setDataType(QAttribute::Float);
        posAttribute->setDataSize(3);
        posAttribute->setAttributeType(QAttribute::VertexAttribute);
        posAttribute->setBuffer(vertexArrayBuffer);
#if 0

        posAttribute->setByteStride(meshData_.getAttributeStride());
        posAttribute->setByteOffset(posAttributeReader.getOffset());
        posAttribute->setCount(meshData_.getNumVertex());
#else
        auto stride = (3+2+3+4) * sizeof(float);
        posAttribute->setByteStride(stride);
        posAttribute->setByteOffset(0);
        posAttribute->setCount(3);
#endif

      /////////
        texCoordAttribute->setName(QAttribute::defaultTextureCoordinateAttributeName());
        texCoordAttribute->setDataType(QAttribute::Float);
        texCoordAttribute->setDataSize(2);
        texCoordAttribute->setAttributeType(QAttribute::VertexAttribute);
        texCoordAttribute->setBuffer(vertexArrayBuffer);
        texCoordAttribute->setByteStride(stride);
        texCoordAttribute->setByteOffset(3 * sizeof(float));
        texCoordAttribute->setCount(3);

        /////////
        normAttribute->setName(QAttribute::defaultNormalAttributeName());
        normAttribute->setDataType(QAttribute::Float);
        normAttribute->setDataSize(3);
        normAttribute->setAttributeType(QAttribute::VertexAttribute);
        normAttribute->setBuffer(vertexArrayBuffer);
        normAttribute->setByteStride(stride);
        normAttribute->setByteOffset(5 * sizeof(float));
        normAttribute->setCount(3);

          /////////
        tanAttribute->setName(QAttribute::defaultTangentAttributeName());
        tanAttribute->setDataType(QAttribute::Float);
        tanAttribute->setDataSize(4);
        tanAttribute->setAttributeType(QAttribute::VertexAttribute);
        tanAttribute->setBuffer(vertexArrayBuffer);
        tanAttribute->setByteStride(stride);
        tanAttribute->setByteOffset(8 * sizeof(float));
        tanAttribute->setCount(3);


        auto indexArrayBuffer = new QBuffer(QBuffer::IndexBuffer, geometry);
#if 0
        {
            auto indexArrayReader = meshData.getIndexArray();
            indexArrayBuffer->setData( QByteArray::fromRawData(indexArrayReader.asChars().begin(), indexArrayReader.size()));
        }
#else
        uint16_t index[6] = {0, 1, 2, 2, 1, 0};
        indexArrayBuffer->setData( QByteArray((const char*)index, 6 * sizeof(uint16_t)));
#endif
        assert( meshData_.getIndexType() == cp::scene::AttributeArrayInterleaved::Type::INT32 );

        auto indexAttribute = new QAttribute(geometry);
        indexAttribute->setAttributeType(QAttribute::IndexAttribute);
        indexAttribute->setDataType(QAttribute::UnsignedShort);
        indexAttribute->setBuffer(indexArrayBuffer);

#if 0
        indexAttribute->setCount(meshData_.getNumIndex());
#else
        indexAttribute->setCount(6);
#endif
        geometry->addAttribute(posAttribute);
        geometry->addAttribute(texCoordAttribute);
        geometry->addAttribute(normAttribute);
        geometry->addAttribute(tanAttribute);
        geometry->addAttribute(indexAttribute);

        QGeometryRenderer::setGeometry(geometry);
    }
#endif
private:
    std::unique_ptr<capnp::MallocMessageBuilder> messageBuilder_;
    cp::scene::AttributeArrayInterleaved::Reader meshData_;
};

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity, std::vector<cp::scene::AttributeArrayInterleaved::Reader> aas)
    : m_rootEntity(rootEntity)
{
//    QFile f("/tmp/shadermesh_assets/f8a4654b-1dbe-52a1-b7c7-2a8c014e3f08");
//    f.open(QFile::ReadOnly);
//    auto mapping = f.map(0, f.size());
//    capnp::ReaderOptions ro;
//    ro.traversalLimitInWords = 1024 * 1024 * 1024;
//    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<capnp::word>((capnp::word *)mapping, f.size() / sizeof(capnp::word)), ro);

//    auto asset = reader.getRoot<cp::asset::Asset>();


    // Torus shape data
    //! [0]
    m_torus = new Qt3DExtras::QTorusMesh();

    m_torus->setRadius(1.0f);
    m_torus->setMinorRadius(0.4f);
    m_torus->setRings(100);
    m_torus->setSlices(20);
    //! [0]

    // TorusMesh Transform
    //! [1]
    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(2.0f);
    torusTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 25.0f));
    torusTransform->setTranslation(QVector3D(5.0f, 4.0f, 0.0f));
    //! [1]

    //! [2]
    Qt3DExtras::QPhongMaterial *torusMaterial = new Qt3DExtras::QPhongMaterial();
    torusMaterial->setDiffuse(QColor(QRgb(0xbeb32b)));
    //! [2]

    // Torus
    //! [3]
    m_torusEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_torusEntity->addComponent(m_torus);
    m_torusEntity->addComponent(torusMaterial);
    m_torusEntity->addComponent(torusTransform);
    //! [3]

    for( auto aaReader : aas )
    {
        // Cone shape data
        auto cone = new CSceneAttributeArrayInterleavedRenderer(aaReader);

        // ConeMesh Transform
        Qt3DCore::QTransform *coneTransform = new Qt3DCore::QTransform();
        //coneTransform->setScale(0.05f);
        //coneTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
        //coneTransform->setTranslation(QVector3D(26.0f, 20.0f, -18.0));

        Qt3DExtras::QPhongMaterial *coneMaterial = new Qt3DExtras::QPhongMaterial();
        coneMaterial->setDiffuse(QColor(QRgb(0x928327)));
        auto coneEntity = new Qt3DCore::QEntity(m_rootEntity);
        coneEntity->addComponent(cone);
        coneEntity->addComponent(coneMaterial);
        coneEntity->addComponent(coneTransform);
    }

    // Cone
    m_coneEntity = new Qt3DCore::QEntity(m_rootEntity);
//    m_coneEntity->addComponent(cone);
//    m_coneEntity->addComponent(coneMaterial);
//    m_coneEntity->addComponent(coneTransform);

    // Cylinder shape data
    Qt3DExtras::QCylinderMesh *cylinder = new Qt3DExtras::QCylinderMesh();
    cylinder->setRadius(1);
    cylinder->setLength(3);
    cylinder->setRings(100);
    cylinder->setSlices(20);

    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform();
    cylinderTransform->setScale(1.5f);
    cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
    cylinderTransform->setTranslation(QVector3D(-5.0f, 4.0f, -1.5));

    Qt3DExtras::QPhongMaterial *cylinderMaterial = new Qt3DExtras::QPhongMaterial();
    cylinderMaterial->setDiffuse(QColor(QRgb(0x928327)));

    // Cylinder
    m_cylinderEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cylinderEntity->addComponent(cylinder);
    m_cylinderEntity->addComponent(cylinderMaterial);
    m_cylinderEntity->addComponent(cylinderTransform);

    // Cuboid shape data
    Qt3DExtras::QCuboidMesh *cuboid = new Qt3DExtras::QCuboidMesh();

    // CuboidMesh Transform
    Qt3DCore::QTransform *cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(4.0f);
    cuboidTransform->setTranslation(QVector3D(5.0f, -4.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *cuboidMaterial = new Qt3DExtras::QPhongMaterial();
    cuboidMaterial->setDiffuse(QColor(QRgb(0x665423)));

    //Cuboid
    m_cuboidEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_cuboidEntity->addComponent(cuboid);
    m_cuboidEntity->addComponent(cuboidMaterial);
    m_cuboidEntity->addComponent(cuboidTransform);

    // Plane shape data
    Qt3DExtras::QPlaneMesh *planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(2);
    planeMesh->setHeight(2);

    // Plane mesh transform
    Qt3DCore::QTransform *planeTransform = new Qt3DCore::QTransform();
    planeTransform->setScale(1.3f);
    planeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
    planeTransform->setTranslation(QVector3D(0.0f, -4.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *planeMaterial = new Qt3DExtras::QPhongMaterial();
    planeMaterial->setDiffuse(QColor(QRgb(0xa69929)));

    // Plane
    m_planeEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_planeEntity->addComponent(planeMesh);
    m_planeEntity->addComponent(planeMaterial);
    m_planeEntity->addComponent(planeTransform);

    // Sphere shape data
    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(2);

    // Sphere mesh transform
    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform();

    sphereTransform->setScale(1.3f);
    sphereTransform->setTranslation(QVector3D(-5.0f, -4.0f, 0.0f));

    Qt3DExtras::QPhongMaterial *sphereMaterial = new Qt3DExtras::QPhongMaterial();
    sphereMaterial->setDiffuse(QColor(QRgb(0xa69929)));

    // Sphere
    m_sphereEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_sphereEntity->addComponent(sphereMesh);
    m_sphereEntity->addComponent(sphereMaterial);
    m_sphereEntity->addComponent(sphereTransform);
}

SceneModifier::~SceneModifier()
{
}

//! [4]
void SceneModifier::enableTorus(bool enabled)
{
    m_torusEntity->setParent(enabled ? m_rootEntity : nullptr);
}
//! [4]

void SceneModifier::enableCone(bool enabled)
{
    m_coneEntity->setParent(enabled ? m_rootEntity : nullptr);
}

void SceneModifier::enableCylinder(bool enabled)
{
    m_cylinderEntity->setParent(enabled ? m_rootEntity : nullptr);
}

void SceneModifier::enableCuboid(bool enabled)
{
    m_cuboidEntity->setParent(enabled ? m_rootEntity : nullptr);
}

void SceneModifier::enablePlane(bool enabled)
{
    m_planeEntity->setParent(enabled ? m_rootEntity : nullptr);
}

void SceneModifier::enableSphere(bool enabled)
{
    m_sphereEntity->setParent(enabled ? m_rootEntity : nullptr);
}
