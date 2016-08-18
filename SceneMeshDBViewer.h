#ifndef SCENEMESHDBVIEWER_H
#define SCENEMESHDBVIEWER_H
#include "GLNavigatable.h"
#include "asset.capnp.h"
#include <capnp/message.h>
#include <QMap>
#include <QFile>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <memory>

class SceneMeshDBViewer
        : public GLNavigatable
        , protected QOpenGLFunctions
{
public:
    SceneMeshDBViewer( cp::asset::AssetMeshData::Reader reader );
    ~SceneMeshDBViewer();

    void draw() override;

private:
    std::unique_ptr<capnp::MallocMessageBuilder> messageBuilder_;
    cp::asset::AssetMeshData::Reader reader_;

    QMap<int, QOpenGLBuffer> elementArrays_;
    QMap<int, QOpenGLBuffer> indexArrays_;
};

#endif // SCENEMESHDBVIEWER_H
