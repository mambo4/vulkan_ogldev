# include "m4_glm_camera.h"
#include "m4_vulkan_utils.h"

GLMCameraFirstPerson::GLMCameraFirstPerson(
    const glm::vec3& pos,
    const glm::vec3& target,
    const glm::vec3& up,
    PerspectiveProjectionInfo& perspectiveProjectionInfo)
{
    Init(pos,target,up,perspectiveProjectionInfo);
}

void GLMCameraFirstPerson::Init(
    const glm::vec3& pos,
    const glm::vec3& target,
    const glm::vec3& up,
    PerspectiveProjectionInfo& perspectiveProjectionInfo)
{
    m_cameraPos=pos;
    m_up=up;
    m_perspectiveProjectionInfo=perspectiveProjectionInfo;

    float aspect=(float) perspectiveProjectionInfo.width/perspectiveProjectionInfo.height;

    if (CAMERA_LEFT_HANDED) {
        m_cameraRot=glm::lookAtLH(pos, target, up);
        m_perspectiveProjection= glm::perspectiveLH_ZO(
            perspectiveProjectionInfo.FOV,
            aspect,
            perspectiveProjectionInfo.zNear,
            perspectiveProjectionInfo.zFar);
    }else{
        m_cameraRot=glm::lookAtRH(pos, target, up);
        m_perspectiveProjection= glm::perspectiveRH_ZO(
            perspectiveProjectionInfo.FOV,
            aspect,
            perspectiveProjectionInfo.zNear,
            perspectiveProjectionInfo.zFar);
    }
}

void GLMCameraFirstPerson::Update(float dt)
{
    if(m_mouseState.m_buttonPressed){
        UpdateCameraOrientation();
    }
    
    m_oldMousePos = m_mouseState.m_pos;

    UpdateVelocity(dt);
    m_cameraPos+= m_velocity*dt;
    
}

void GLMCameraFirstPerson::SetMousePos( float xpos, float ypos )
{
    float new_x=( xpos / (float)m_perspectiveProjectionInfo.width);
    float new_y=(ypos / (float)m_perspectiveProjectionInfo.height);
    glm::vec2 new_pos=glm::vec2(new_x,new_y);
    m_mouseState.m_pos=new_pos;
}


void GLMCameraFirstPerson::HandleMouseButton(int button, int action, int mods)
{
    if (button==GLFW_MOUSE_BUTTON_LEFT){
        m_mouseState.m_buttonPressed= (action==GLFW_PRESS);
    }
}

void GLMCameraFirstPerson::UpdateCameraOrientation(){
    glm::vec2 deltaMouse=m_mouseState.m_pos-m_oldMousePos;
    glm::quat deltaRot= glm::quat(glm::vec3(m_mouseSpeed*deltaMouse.y,m_mouseSpeed*deltaMouse.x,0.0f));
    m_cameraRot=glm::normalize(deltaRot*m_cameraRot);
    SetUpVector();
}

void GLMCameraFirstPerson::UpdateVelocity(float dt)
{
    
    glm::vec3 acceleration= GetAcceleration();

    if(acceleration==glm::vec3(0.0f)){
        m_velocity -= m_velocity * std::min(dt*m_damping, 1.0f);
    }else{
        m_velocity += acceleration*m_acceleration*dt;
        float maxSpeed= m_movement.fast ? m_maxSpeed*m_fastCoefficient : m_maxSpeed;
        if (glm::length(m_velocity) > m_maxSpeed){
            m_velocity= glm::normalize(m_velocity)*m_maxSpeed;
        }
    }


}

glm::vec3 GLMCameraFirstPerson::GetAcceleration()
{

    glm::mat4 v = glm::mat4_cast(m_cameraRot);
    glm::vec3 forward = glm::vec3(v[0][2], v[1][2], v[2][2]) * 0.1f;
    glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);
    glm::vec3 up=glm::cross(forward,right);

    if(!CAMERA_LEFT_HANDED){
        forward = - glm::vec3(v[0][2], v[1][2], v[2][2]) * 0.1f;
        up=glm::cross(right, forward);
    }
    
    glm::vec3 acceleration= glm::vec3(0.0f);

    if (m_movement.forward){acceleration += forward;}
    if (m_movement.back){acceleration -=forward;}
    if (m_movement.right){acceleration += right;}
    if (m_movement.left){acceleration -=right;}
    if (m_movement.up){acceleration -= up;}//vulkan reversed Y
    if (m_movement.down){ acceleration += up; }//vulkan reversed Y
    if(m_movement.fast){acceleration*=m_fastCoefficient;}

    /*
    to implement plus and minus to speed see
    https://github.com/emeiri/ogldev/blob/master/Common/ogldev_glm_camera.cpp#L96
    */

    return acceleration;
}

glm::mat4 GLMCameraFirstPerson::GetViewMatrix() const
{
    glm::mat4 t= glm::translate(glm::mat4(1.0), -m_cameraPos);
    glm::mat4 r= glm::mat4_cast(m_cameraRot);
    glm::mat4 result=r*t;
    return result;
}

glm::mat4 GLMCameraFirstPerson::GetVPMatrix() const
{
    glm::mat4 view=GetViewMatrix();
    glm::mat4 VP=m_perspectiveProjection* view;
    return VP;
}

glm::mat4 GLMCameraFirstPerson::GetVPMatrixNoTranslate() const
{
    glm::mat4 view=glm::mat4_cast(m_cameraRot);
    glm::mat4 VP=m_perspectiveProjection* view;
    return VP;
}

void GLMCameraFirstPerson::SetUpVector(){
    glm::mat4 view = GetViewMatrix();
    glm::vec3 forward =glm::vec3(view[0][2], view[1][2], view[2][2]);

    if(CAMERA_LEFT_HANDED){
        m_cameraRot=glm::lookAtLH(m_cameraPos,m_cameraPos+forward,m_up);
    }else{
        m_cameraRot=glm::lookAtRH(m_cameraPos,m_cameraPos-forward,m_up);
    }
}

void GLMCameraFirstPerson::SetTarget(const glm::vec3& Target)
{
	SetAbsTarget(m_cameraPos + Target);
}

void GLMCameraFirstPerson::SetAbsTarget(const glm::vec3& Target)
{
	if (CAMERA_LEFT_HANDED) {
		m_cameraRot = glm::lookAtLH(m_cameraPos, Target, m_up);
	}
	else {
		m_cameraRot = glm::lookAtRH(m_cameraPos, Target, m_up);
	}
}

glm::vec3 GLMCameraFirstPerson::GetTarget() const
{
	glm::mat4 view = GetViewMatrix();
	glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);
	return forward;
}

bool GLFWCameraHandler(CameraMovement& movement, int key, int action, int mods)
{

    bool press = action!=GLFW_RELEASE;
    bool handled =true;

    switch(key)
    {
        case GLFW_KEY_W:
            movement.forward=press;
            break;
        case GLFW_KEY_A:
            movement.left=press;
            break;
        case GLFW_KEY_S:
            movement.back=press;
            break;
        case GLFW_KEY_D:
            movement.right=press;
            break;
        case GLFW_KEY_PAGE_UP:
            movement.up=press;
            break;
        case GLFW_KEY_PAGE_DOWN:
            movement.down=press;
            break;
        default:
            handled=false;
    }
    

    if (mods & GLFW_MOD_SHIFT) {
        movement.fast = press;
    }

    return handled;
}