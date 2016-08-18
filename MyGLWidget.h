#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H
#include "GLNavigatable.h"

#include <qopenglbuffer.h>
#include <qopenglfunctions.h>
#include <qopenglwidget.h>

#include <QFile>
#include <QPointF>
#include <QVector3D>

#include <memory>

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MyGLWidget(QWidget *parent);
    ~MyGLWidget();

    void setNavigatable(std::unique_ptr<GLNavigatable> navigatable);

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void updateMove();

private slots:
    void on_moveTimer_timeout();

private:
    // QOpenGl

    bool &getKey(const Qt::Key &key);
    bool anyKeyPressed() const;

    float lon_;
    float lat_;
    QPointF lastMousePos_;
    bool mousePressed_;

    bool forwardPressed_;
    bool backwardPressed_;
    bool rightPressed_;
    bool leftPressed_;

    bool nullPressed_;

    QTimer *moveTimer;

    QVector3D forwardVelocity_;
    QVector3D rightVelocity_;

    QVector3D pos_;

    bool stateChanged_;
    QScopedPointer<GLNavigatable> navigatable_;
};

#endif // MYGLWIDGET_H
