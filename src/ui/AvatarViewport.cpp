#include "ui/AvatarViewport.h"
#include "avatar/AvatarEngine.h"
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QElapsedTimer>
#include <QColor>
#include <spdlog/spdlog.h>

namespace Chatbot {

AvatarViewport::AvatarViewport(QWidget *parent)
    : QWidget(parent)
    , m_view(std::make_unique<Qt3DExtras::Qt3DWindow>())
    , m_container(nullptr)
    , m_rootEntity(nullptr)
    , m_camera(nullptr)
    , m_renderer(nullptr)
    , m_avatarEngine(nullptr)
    , m_animationTimer(nullptr)
    , m_lastFrameTime(0)
{
    spdlog::info("AvatarViewport initializing...");

    // Set background color
    m_view->defaultFrameGraph()->setClearColor(QColor(200, 220, 240));  // Light blue/gray

    // Create the 3D scene
    setup3DScene();

    // Embed Qt3D window in this widget
    m_container = QWidget::createWindowContainer(m_view.get(), this);
    m_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_container);
    setLayout(layout);

    // Set minimum size
    setMinimumSize(400, 400);

    // Start animation timer
    startAnimationTimer();

    spdlog::info("AvatarViewport initialized");
}

AvatarViewport::~AvatarViewport() {
    if (m_animationTimer) {
        m_animationTimer->stop();
    }
    spdlog::info("AvatarViewport destroyed");
}

void AvatarViewport::setup3DScene() {
    // Create root entity
    m_rootEntity = new Qt3DCore::QEntity();

    // Setup camera
    setupCamera();

    // Setup lighting
    setupLighting();

    // Create avatar engine
    m_avatarEngine = std::make_unique<AvatarEngine>(m_rootEntity, this);
    m_avatarEngine->setState(AvatarState::Idle);
    m_avatarEngine->startIdleAnimation();  // Explicitly start animation

    // Set the root entity (must be done after creating all entities)
    m_view->setRootEntity(m_rootEntity);

    spdlog::debug("3D scene setup complete");
    spdlog::info("Avatar viewport initialized with 3D scene");
}

void AvatarViewport::setupCamera() {
    m_camera = m_view->camera();

    // Position camera to look at avatar
    m_camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    m_camera->setPosition(QVector3D(0.0f, 1.0f, 3.0f));
    m_camera->setViewCenter(QVector3D(0.0f, 1.0f, 0.0f));
    m_camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    spdlog::debug("Camera setup complete");
}

void AvatarViewport::setupLighting() {
    // Create main light (key light from front-right)
    Qt3DCore::QEntity* lightEntity1 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight* light1 = new Qt3DRender::QPointLight(lightEntity1);
    light1->setColor(QColor(255, 250, 240));  // Warm white
    light1->setIntensity(0.7f);  // Reduced intensity

    Qt3DCore::QTransform* lightTransform1 = new Qt3DCore::QTransform();
    lightTransform1->setTranslation(QVector3D(2.0f, 3.0f, 2.0f));

    lightEntity1->addComponent(light1);
    lightEntity1->addComponent(lightTransform1);

    // Create fill light (from left, softer)
    Qt3DCore::QEntity* lightEntity2 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight* light2 = new Qt3DRender::QPointLight(lightEntity2);
    light2->setColor(QColor(240, 245, 255));  // Cool white
    light2->setIntensity(0.4f);  // Softer fill

    Qt3DCore::QTransform* lightTransform2 = new Qt3DCore::QTransform();
    lightTransform2->setTranslation(QVector3D(-2.0f, 2.0f, 1.0f));

    lightEntity2->addComponent(light2);
    lightEntity2->addComponent(lightTransform2);

    // Create back light (rim light)
    Qt3DCore::QEntity* lightEntity3 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight* light3 = new Qt3DRender::QPointLight(lightEntity3);
    light3->setColor(QColor(200, 220, 255));  // Blue rim
    light3->setIntensity(0.2f);  // Subtle rim

    Qt3DCore::QTransform* lightTransform3 = new Qt3DCore::QTransform();
    lightTransform3->setTranslation(QVector3D(0.0f, 1.5f, -2.0f));

    lightEntity3->addComponent(light3);
    lightEntity3->addComponent(lightTransform3);

    spdlog::debug("Lighting setup complete (3-point lighting with reduced intensity)");
}

void AvatarViewport::startAnimationTimer() {
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &AvatarViewport::onAnimationTick);

    // 60 FPS target
    m_animationTimer->start(16);  // ~60 FPS (16ms per frame)

    m_lastFrameTime = QDateTime::currentMSecsSinceEpoch();

    spdlog::debug("Animation timer started (target: 60 FPS)");
}

void AvatarViewport::onAnimationTick() {
    // Calculate delta time
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;  // Convert to seconds
    m_lastFrameTime = currentTime;

    // Update avatar animation
    if (m_avatarEngine) {
        m_avatarEngine->updateAnimation(deltaTime);
    }
}

} // namespace Chatbot
