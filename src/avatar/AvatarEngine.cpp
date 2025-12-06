#include "avatar/AvatarEngine.h"
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <QColor>
#include <QtMath>
#include <spdlog/spdlog.h>

namespace Chatbot {

AvatarEngine::AvatarEngine(Qt3DCore::QEntity *rootEntity, QObject *parent)
    : QObject(parent)
    , m_rootEntity(rootEntity)
    , m_avatarEntity(nullptr)
    , m_headMesh(nullptr)
    , m_neckMesh(nullptr)
    , m_material(nullptr)
    , m_headTransform(nullptr)
    , m_neckTransform(nullptr)
    , m_mouthMesh(nullptr)
    , m_mouthTransform(nullptr)
    , m_mouthMaterial(nullptr)
    , m_state(AvatarState::Idle)
    , m_animationTime(0.0f)
    , m_animationSpeed(1.0f)
    , m_isAnimating(false)
    , m_visemeMapper(std::make_unique<VisemeMapper>())
    , m_visemeBlendTime(0.0f)
    , m_visemeBlendDuration(0.05f)  // 50ms blend time
{
    spdlog::info("AvatarEngine initializing...");

    // Load viseme mapping
    m_visemeMapper->loadMapping("./config/viseme_mapping.json");
    m_currentViseme = m_visemeMapper->getSilenceViseme();
    m_targetViseme = m_currentViseme;

    createPlaceholderAvatar();
    setupIdleAnimation();
    spdlog::info("AvatarEngine initialized");
}

AvatarEngine::~AvatarEngine() {
    spdlog::info("AvatarEngine destroyed");
}

void AvatarEngine::createPlaceholderAvatar() {
    spdlog::info("Creating placeholder avatar...");

    // Create main avatar entity
    m_avatarEntity = new Qt3DCore::QEntity(m_rootEntity);

    // Create head (sphere)
    m_headMesh = new Qt3DExtras::QSphereMesh();
    m_headMesh->setRadius(0.5f);
    m_headMesh->setRings(32);
    m_headMesh->setSlices(32);

    m_headTransform = new Qt3DCore::QTransform();
    m_headTransform->setTranslation(QVector3D(0.0f, 1.0f, 0.0f));

    // Create neck (cylinder)
    m_neckMesh = new Qt3DExtras::QCylinderMesh();
    m_neckMesh->setRadius(0.15f);
    m_neckMesh->setLength(0.4f);
    m_neckMesh->setRings(16);
    m_neckMesh->setSlices(16);

    m_neckTransform = new Qt3DCore::QTransform();
    m_neckTransform->setTranslation(QVector3D(0.0f, 0.5f, 0.0f));

    // Create materials (separate for head and neck to be visible)
    Qt3DExtras::QPhongMaterial* headMaterial = new Qt3DExtras::QPhongMaterial();
    headMaterial->setDiffuse(QColor(230, 180, 140));  // Peach/tan skin tone
    headMaterial->setAmbient(QColor(180, 140, 100));  // Darker ambient for depth
    headMaterial->setSpecular(QColor(50, 50, 50));    // Less specular (less shiny)
    headMaterial->setShininess(10.0f);

    Qt3DExtras::QPhongMaterial* neckMaterial = new Qt3DExtras::QPhongMaterial();
    neckMaterial->setDiffuse(QColor(200, 160, 120));  // Slightly darker for neck
    neckMaterial->setAmbient(QColor(160, 120, 80));
    neckMaterial->setSpecular(QColor(50, 50, 50));
    neckMaterial->setShininess(10.0f);

    // Create head entity
    Qt3DCore::QEntity* headEntity = new Qt3DCore::QEntity(m_avatarEntity);
    headEntity->addComponent(m_headMesh);
    headEntity->addComponent(m_headTransform);
    headEntity->addComponent(headMaterial);

    // Create neck entity
    Qt3DCore::QEntity* neckEntity = new Qt3DCore::QEntity(m_avatarEntity);
    neckEntity->addComponent(m_neckMesh);
    neckEntity->addComponent(m_neckTransform);
    neckEntity->addComponent(neckMaterial);

    // Create mouth (flattened sphere on front of head)
    m_mouthMesh = new Qt3DExtras::QSphereMesh();
    m_mouthMesh->setRadius(0.08f);  // Small mouth
    m_mouthMesh->setRings(16);
    m_mouthMesh->setSlices(16);

    m_mouthTransform = new Qt3DCore::QTransform();
    // Position relative to head center (head is a sphere at origin of headEntity)
    // Head center is at (0, 0, 0) in head-local space
    // Place mouth slightly below center and in front
    m_mouthTransform->setTranslation(QVector3D(0.0f, -0.1f, 0.45f));
    // Flatten in Z to make it look more like a mouth shape
    m_mouthTransform->setScale3D(QVector3D(1.0f, 0.6f, 0.3f));

    m_mouthMaterial = new Qt3DExtras::QPhongMaterial();
    m_mouthMaterial->setDiffuse(QColor(180, 100, 100));  // Reddish for mouth/lips
    m_mouthMaterial->setAmbient(QColor(120, 60, 60));
    m_mouthMaterial->setSpecular(QColor(30, 30, 30));
    m_mouthMaterial->setShininess(15.0f);

    // Create mouth entity as child of HEAD (not avatar) so it moves with head
    Qt3DCore::QEntity* mouthEntity = new Qt3DCore::QEntity(headEntity);
    mouthEntity->addComponent(m_mouthMesh);
    mouthEntity->addComponent(m_mouthTransform);
    mouthEntity->addComponent(m_mouthMaterial);

    spdlog::info("Placeholder avatar created: head at (0, 1, 0), neck at (0, 0.5, 0), mouth at (0, 0.9, 0.45)");
    spdlog::debug("Head: sphere radius=0.5, Neck: cylinder radius=0.15 length=0.4, Mouth: flattened sphere");
}

void AvatarEngine::setupIdleAnimation() {
    // Idle animation will be driven by updateAnimation() slot
    spdlog::debug("Idle animation setup complete");
}

void AvatarEngine::setState(AvatarState state) {
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);

        spdlog::info("Avatar state changed to: {}", static_cast<int>(state));

        // Start/stop animations based on state
        if (state == AvatarState::Idle) {
            startIdleAnimation();
        } else if (state == AvatarState::Speaking) {
            // Speaking animation will be added in Phase 4
            spdlog::debug("Speaking state - lip-sync will be implemented in Phase 4");
        }
    }
}

void AvatarEngine::startIdleAnimation() {
    m_isAnimating = true;
    m_animationTime = 0.0f;
    spdlog::debug("Idle animation started");
}

void AvatarEngine::stopIdleAnimation() {
    m_isAnimating = false;
    spdlog::debug("Idle animation stopped");
}

void AvatarEngine::setAnimationSpeed(float speed) {
    m_animationSpeed = speed;
    spdlog::debug("Animation speed set to: {}", speed);
}

void AvatarEngine::updateAnimation(float deltaTime) {
    if (!m_isAnimating || m_state != AvatarState::Idle) {
        return;
    }

    m_animationTime += deltaTime * m_animationSpeed;

    // Gentle bobbing motion (up and down)
    float bobAmount = 0.03f;  // Very subtle vertical movement
    float bobFrequency = 1.0f;  // Hz (slow, breathing-like rhythm)
    float yOffset = bobAmount * qSin(m_animationTime * bobFrequency * 2.0f * M_PI);

    // Gentle head rotation (subtle nod)
    float rotationAmount = 8.0f;  // degrees (more natural, less exaggerated)
    float rotationFrequency = 0.6f;  // Hz (slower, calmer)
    float rotation = rotationAmount * qSin(m_animationTime * rotationFrequency * 2.0f * M_PI);

    // Apply transformations
    QVector3D basePosition(0.0f, 1.0f, 0.0f);
    m_headTransform->setTranslation(basePosition + QVector3D(0.0f, yOffset, 0.0f));

    // Subtle rotation around X axis (nodding)
    m_headTransform->setRotationX(rotation);
}

void AvatarEngine::setPosition(const QVector3D& position) {
    if (m_avatarEntity) {
        // Position will be handled by a transform component on the avatar entity
        spdlog::debug("Avatar position set to: ({}, {}, {})",
                     position.x(), position.y(), position.z());
    }
}

void AvatarEngine::setRotation(const QQuaternion& rotation) {
    if (m_avatarEntity) {
        spdlog::debug("Avatar rotation set");
    }
}

void AvatarEngine::setScale(float scale) {
    if (m_avatarEntity) {
        spdlog::debug("Avatar scale set to: {}", scale);
    }
}

bool AvatarEngine::loadModel(const QString& modelPath) {
    spdlog::info("Model loading requested: {}", modelPath.toStdString());
    spdlog::warn("GLTF model loading not yet implemented - using placeholder avatar");
    emit modelLoadFailed("GLTF loading will be implemented in future update");
    return false;

    // TODO: Implement GLTF loading using Qt3D's scene loader
    // This will be enhanced once we have a proper model with blend shapes
}

void AvatarEngine::applyViseme(const Viseme& viseme, float blendFactor) {
    if (!m_mouthTransform) {
        return;
    }

    // Set target viseme for blending
    m_targetViseme = viseme;

    // If immediate application (blendFactor = 1.0), skip blending
    if (blendFactor >= 1.0f) {
        m_currentViseme = viseme;
        m_visemeBlendTime = m_visemeBlendDuration;
    } else {
        // Start blending
        m_visemeBlendTime = 0.0f;
    }

    // Apply mouth shape based on viseme parameters
    // Scale the mouth mesh to match the viseme's mouth width and height
    float baseScale = 1.0f;
    float width = baseScale + (viseme.mouthWidth * 2.0f);   // Scale width
    float height = baseScale + (viseme.mouthHeight * 2.0f); // Scale height
    float depth = 0.3f;  // Keep depth relatively constant (flat mouth)

    // Apply jaw opening by translating mouth down
    float jawOffset = viseme.jawOpen * 0.1f;  // Move down when jaw opens

    // Update mouth transform (position relative to head center)
    QVector3D basePosition(0.0f, -0.1f, 0.45f);  // Head-relative coordinates
    m_mouthTransform->setTranslation(basePosition + QVector3D(0.0f, -jawOffset, 0.0f));
    m_mouthTransform->setScale3D(QVector3D(width, height, depth));

    spdlog::debug("Applied viseme: {} (width={:.2f}, height={:.2f}, jaw={:.2f})",
                  viseme.name.toStdString(), viseme.mouthWidth, viseme.mouthHeight, viseme.jawOpen);
}

void AvatarEngine::applyPhoneme(const QString& phoneme) {
    if (!m_visemeMapper || !m_visemeMapper->isLoaded()) {
        spdlog::warn("VisemeMapper not loaded, cannot apply phoneme: {}", phoneme.toStdString());
        return;
    }

    // Convert phoneme to viseme
    Viseme viseme = m_visemeMapper->getVisemeForPhoneme(phoneme);

    // Apply the viseme
    applyViseme(viseme, 0.5f);  // Use blending

    spdlog::debug("Phoneme '{}' mapped to viseme '{}'",
                  phoneme.toStdString(), viseme.name.toStdString());
}

} // namespace Chatbot
