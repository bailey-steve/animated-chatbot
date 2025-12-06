#ifndef CHATBOT_AVATARENGINE_H
#define CHATBOT_AVATARENGINE_H

#include "avatar/VisemeMapper.h"
#include <QObject>
#include <QString>
#include <QVector3D>
#include <QQuaternion>
#include <memory>

// Forward declarations for Qt3D
namespace Qt3DCore {
    class QEntity;
    class QTransform;
}

namespace Qt3DExtras {
    class QSphereMesh;
    class QCylinderMesh;
    class QPhongMaterial;
}

namespace Qt3DAnimation {
    class QAnimationController;
}

namespace Chatbot {

// Avatar animation state
enum class AvatarState {
    Idle,
    Speaking,
    Listening
};

class AvatarEngine : public QObject {
    Q_OBJECT

public:
    explicit AvatarEngine(Qt3DCore::QEntity *rootEntity, QObject *parent = nullptr);
    ~AvatarEngine() override;

    // Delete copy constructor and assignment operator
    AvatarEngine(const AvatarEngine&) = delete;
    AvatarEngine& operator=(const AvatarEngine&) = delete;

    // Avatar control
    void setState(AvatarState state);
    AvatarState getState() const { return m_state; }

    // Model loading (for future GLTF support)
    bool loadModel(const QString& modelPath);

    // Animation control
    void startIdleAnimation();
    void stopIdleAnimation();
    void setAnimationSpeed(float speed);

    // Position and orientation
    void setPosition(const QVector3D& position);
    void setRotation(const QQuaternion& rotation);
    void setScale(float scale);

    // Lip-sync / Viseme control
    void applyViseme(const Viseme& viseme, float blendFactor = 1.0f);
    void applyPhoneme(const QString& phoneme);

signals:
    void modelLoaded(const QString& modelPath);
    void modelLoadFailed(const QString& error);
    void stateChanged(AvatarState newState);

public slots:
    void updateAnimation(float deltaTime);

private:
    void createPlaceholderAvatar();
    void setupIdleAnimation();

private:
    Qt3DCore::QEntity* m_rootEntity;
    Qt3DCore::QEntity* m_avatarEntity;

    // Placeholder avatar components (simple sphere for head)
    Qt3DExtras::QSphereMesh* m_headMesh;
    Qt3DExtras::QCylinderMesh* m_neckMesh;
    Qt3DExtras::QPhongMaterial* m_material;
    Qt3DCore::QTransform* m_headTransform;
    Qt3DCore::QTransform* m_neckTransform;

    // Mouth components
    Qt3DExtras::QSphereMesh* m_mouthMesh;
    Qt3DCore::QTransform* m_mouthTransform;
    Qt3DExtras::QPhongMaterial* m_mouthMaterial;

    // Animation
    AvatarState m_state;
    float m_animationTime;
    float m_animationSpeed;
    bool m_isAnimating;

    // Lip-sync / Viseme
    std::unique_ptr<VisemeMapper> m_visemeMapper;
    Viseme m_currentViseme;
    Viseme m_targetViseme;
    float m_visemeBlendTime;
    float m_visemeBlendDuration;
};

} // namespace Chatbot

#endif // CHATBOT_AVATARENGINE_H
