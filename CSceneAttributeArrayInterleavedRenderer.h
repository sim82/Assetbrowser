#pragma once
#include "scene.capnp.h"

#include <Qt3DRender/QGeometryRenderer>
#include <capnp/message.h>
#include <memory>

class QNode;
namespace Qt3DRender
{
class QBuffer;
}

class CSceneAttributeArrayInterleavedRenderer : public Qt3DRender::QGeometryRenderer
{

    std::vector<Qt3DRender::QAttribute *> createAttributes(cp::scene::AttributeArrayInterleaved::Reader aa,
                                                           Qt3DRender::QGeometry *geometry,
                                                           Qt3DRender::QBuffer *buffer);

public:
    CSceneAttributeArrayInterleavedRenderer(cp::scene::AttributeArrayInterleaved::Reader meshData,
                                            QNode *parent = nullptr);
    ~CSceneAttributeArrayInterleavedRenderer();

private:
    std::unique_ptr<capnp::MallocMessageBuilder> messageBuilder_;
    cp::scene::AttributeArrayInterleaved::Reader meshData_;
};
