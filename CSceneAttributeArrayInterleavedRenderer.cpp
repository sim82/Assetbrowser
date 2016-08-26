#include "CSceneAttributeArrayInterleavedRenderer.h"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <QtCore/QDebug>
#include <QBuffer>
#include <QAttribute>
#include <QFile>
#include <memory>
#include <cassert>
#include <iostream>


std::vector<Qt3DRender::QAttribute *>
CSceneAttributeArrayInterleavedRenderer::createAttributes(cp::scene::AttributeArrayInterleaved::Reader aa,
                                                          Qt3DRender::QGeometry *geometry, Qt3DRender::QBuffer *buffer)
{
    using namespace Qt3DRender;
    std::vector<QAttribute *> attributes;

    std::map<std::string, QString> nameMap({{"a_position", QAttribute::defaultPositionAttributeName()},
                                            {"a_texcoord", QAttribute::defaultTextureCoordinateAttributeName()},
                                            {"a_normal", QAttribute::defaultNormalAttributeName()},
                                            {"a_t", QAttribute::defaultTangentAttributeName()}});

    for (auto attribute : aa.getAttributes())
    {
        auto it = nameMap.find(attribute.getName());
        if (it == nameMap.end())
        {
            continue;
        }

        assert(attribute.getType() == cp::scene::AttributeArrayInterleaved::Type::FLOAT32);

        auto qAttribute = new QAttribute(geometry);
        auto name       = it->second;
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

CSceneAttributeArrayInterleavedRenderer::CSceneAttributeArrayInterleavedRenderer(
    cp::scene::AttributeArrayInterleaved::Reader meshData, QNode *parent)
    : Qt3DRender::QGeometryRenderer(parent)
    , messageBuilder_(std::make_unique<capnp::MallocMessageBuilder>())
{
    messageBuilder_->setRoot(meshData);
    meshData_ = messageBuilder_->getRoot<cp::scene::AttributeArrayInterleaved>();

    using namespace Qt3DRender;

    auto geometry = new QGeometry(this);

    auto vertexArrayBuffer = new QBuffer(QBuffer::VertexBuffer, geometry);

    auto vertexArrayReader = meshData_.getAttributeArray();
    vertexArrayBuffer->setData(QByteArray::fromRawData(vertexArrayReader.asChars().begin(), vertexArrayReader.size()));
    auto qAttributes = createAttributes(meshData_, geometry, vertexArrayBuffer);

    auto indexArrayBuffer = new QBuffer(QBuffer::IndexBuffer, geometry);
    {
        auto indexArrayReader = meshData_.getIndexArray();
        indexArrayBuffer->setData(QByteArray::fromRawData(indexArrayReader.asChars().begin(), indexArrayReader.size()));
    }
    assert(meshData_.getIndexType() == cp::scene::AttributeArrayInterleaved::Type::INT32);

    auto indexAttribute = new QAttribute(geometry);
    indexAttribute->setAttributeType(QAttribute::IndexAttribute);
    indexAttribute->setDataType(QAttribute::UnsignedInt);
    indexAttribute->setBuffer(indexArrayBuffer);

    indexAttribute->setCount(meshData_.getNumIndex());

    for (auto qa : qAttributes)
    {
        geometry->addAttribute(qa);
    }
    geometry->addAttribute(indexAttribute);
    QGeometryRenderer::setPrimitiveType(QGeometryRenderer::Triangles);
    QGeometryRenderer::setGeometry(geometry);

}

CSceneAttributeArrayInterleavedRenderer::~CSceneAttributeArrayInterleavedRenderer()
{
    std::cerr << "~CSceneAttributeArrayInterleavedRenderer()" << std::endl;
}
