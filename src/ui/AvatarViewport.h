#ifndef CHATBOT_AVATARVIEWPORT_H
#define CHATBOT_AVATARVIEWPORT_H

#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <memory>

// Forward declarations
namespace Qt3DCore {
    class QEntity;
}

namespace Qt3DRender {
    class QCamera;
}

namespace Qt3DExtras {
    class QForwardRenderer;
    class QFirstPersonCameraController;
}

namespace Chatbot {

class AvatarEngine;

class AvatarViewport : public QWidget {
    Q_OBJECT

public:
    explicit AvatarViewport(QWidget *parent = nullptr);
    ~AvatarViewport() override;

    // Delete copy constructor and assignment operator
    AvatarViewport(const AvatarViewport&) = delete;
    AvatarViewport& operator=(const AvatarViewport&) = delete;

    // Get the avatar engine for external control
    AvatarEngine* getAvatarEngine() const { return m_avatarEngine.get(); }

private:
    void setup3DScene();
    void setupCamera();
    void setupLighting();
    void startAnimationTimer();

private slots:
    void onAnimationTick();

private:
    // Qt3D components
    std::unique_ptr<Qt3DExtras::Qt3DWindow> m_view;
    QWidget* m_container;

    Qt3DCore::QEntity* m_rootEntity;
    Qt3DRender::QCamera* m_camera;
    Qt3DExtras::QForwardRenderer* m_renderer;

    // Avatar engine
    std::unique_ptr<AvatarEngine> m_avatarEngine;

    // Animation
    QTimer* m_animationTimer;
    qint64 m_lastFrameTime;
};

} // namespace Chatbot

#endif // CHATBOT_AVATARVIEWPORT_H
