#include "RenderingOrderExp.h"

namespace INANOA {	

	// ===========================================================
	RenderingOrderExp::RenderingOrderExp(){
		this->m_cameraForwardSpeed = 0.25f;
		this->m_cameraForwardMagnitude = glm::vec3(0.0f, 0.0f, 0.0f);
		this->m_frameWidth = 64;
		this->m_frameHeight = 64;
	}
	RenderingOrderExp::~RenderingOrderExp(){}

	bool RenderingOrderExp::init(const int w, const int h) {
		INANOA::OPENGL::RendererBase* renderer = new INANOA::OPENGL::RendererBase();
		const std::string vsFile = "src\\shader\\vertexShader_ogl_450.glsl";
		const std::string fsFile = "src\\shader\\fragmentShader_ogl_450.glsl";
		if (renderer->init(vsFile, fsFile, w, h) == false) {
			return false;
		}

		this->m_renderer = renderer;
		myObject = new Object("models/foliages/bush01_lod2.obj", this->m_renderer->getId());

		this->m_godCamera = new Camera(
			glm::vec3(0.0f, 0.0f, 2.0f), // view org
			glm::vec3(0.0f, 0.0f, 0.0f), // look at
			glm::vec3(0.0f, 1.0f, 0.0f), // up vector
			5.0f,	// distance 
			60.0f,	// fov
			0.1f,	// near
			512.0f	// far
		);
		this->m_godCamera->resize(w, h);

		this->m_godCamera->setViewOrg(glm::vec3(0.0f, 55.0f, 50.0f));
		this->m_godCamera->setLookCenter(glm::vec3(0.0f, 32.0f, -12.0f));
		this->m_godCamera->setDistance(70.0f);
		this->m_godCamera->update();

		this->m_playerCamera = new Camera(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 9.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0, 45.0f, 1.0f, 150.0f);
		this->m_playerCamera->resize(w, h);
		this->m_playerCamera->update();

		m_renderer->setCamera(
			this->m_godCamera->projMatrix(),
			this->m_godCamera->viewMatrix(),
			this->m_godCamera->viewOrig()
		);

		// view frustum and horizontal ground
		{
			this->m_viewFrustum = new SCENE::RViewFrustum(1, nullptr);
			this->m_viewFrustum->resize(this->m_playerCamera);

			this->m_horizontalGround = new SCENE::EXPERIMENTAL::HorizonGround(2, nullptr);
			this->m_horizontalGround->resize(this->m_playerCamera);
		}

		this->resize(w, h);		
		return true;
	}
	void RenderingOrderExp::resize(const int w, const int h) {
		const int HW = w * 0.5;

		this->m_playerCamera->resize(HW, h);
		this->m_godCamera->resize(HW, h);
		m_renderer->resize(w, h);
		this->m_frameWidth = w;
		this->m_frameHeight = h;

		this->m_viewFrustum->resize(this->m_playerCamera);
		this->m_horizontalGround->resize(this->m_playerCamera);
	}
	void RenderingOrderExp::update(double dT) {
		static bool firstClick = true;

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		static double lastxpos, lastypos;
		double dY = 0.0, dX = 0.0;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (firstClick) {
				lastypos = ypos;
				lastxpos = xpos;
				firstClick = false;
			}
			else {
				dY = ypos - lastypos;
				dX = xpos - lastxpos;

				lastypos = ypos;
				lastxpos = xpos;
			}
		}
		else {
			firstClick = true;
			dY = dX = 0.0f;
		}

		// Movement
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			this->m_godCamera->translate(2.0f * dT);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			this->m_godCamera->translate(-2.0f * dT);
		}
		this->m_godCamera->rotate(dX * 0.5f * (float)dT);
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			this->m_godCamera->fly(100.0f * (float)dT);	
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			this->m_godCamera->fly(-100.0f * (float)dT);
		}


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			this->m_playerCamera->translate(2.0f * dT);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			this->m_playerCamera->translate(-2.0f * dT);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			this->m_playerCamera->rotate(1.0f * (float)dT);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			this->m_playerCamera->rotate(-1.0f * (float)dT);
		}

		// camera update (god)
		m_godCamera->update();

		// camera update (player)
		this->m_playerCamera->forward(this->m_cameraForwardMagnitude, true);
		this->m_playerCamera->update();

		// lock to view space
		this->m_viewFrustum->update(this->m_playerCamera);
		this->m_horizontalGround->update(this->m_playerCamera);

		myObject->update(dT);
	}

	void RenderingOrderExp::render() {		
		this->m_renderer->useProgram();
		this->m_renderer->clearRenderTarget();
		const int HW = this->m_frameWidth * 0.5;

		// =====================================================
		// god view
		this->m_renderer->setCamera(
			m_godCamera->projMatrix(),
			m_godCamera->viewMatrix(),
			m_godCamera->viewOrig()
		);

		this->m_renderer->setViewport(0, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::UNLIT);
		this->m_viewFrustum->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::MESH);
		glm::mat4 mat = this->m_playerCamera->projMatrix() * this->m_playerCamera->viewMatrix();
		myObject->draw(m_renderer, mat, true);

		// =====================================================
		// player view
		this->m_renderer->useProgram();
		this->m_renderer->clearDepth();
		this->m_renderer->setCamera(
			this->m_playerCamera->projMatrix(),
			this->m_playerCamera->viewMatrix(),
			this->m_playerCamera->viewOrig()
		);

		this->m_renderer->setViewport(HW, 0, HW, this->m_frameHeight);
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::PROCEDURAL_GRID);
		this->m_horizontalGround->render();	
		this->m_renderer->setShadingModel(OPENGL::ShadingModelType::MESH);
		myObject->draw(this->m_renderer, mat, false);
	}
}
