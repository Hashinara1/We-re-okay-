#include "pch.h"
#include "Eyes.h"

//Rouge = 212, 13, 13
//Jaune = 250, 168, 45
//Bleu = 52, 146, 235

Eyes::Eyes()
{
    m_pEyeLeft = nullptr;
    m_pEyeRight = nullptr;
    m_pPupilLeft = nullptr;
    m_pPupilRight = nullptr;
    m_pEyelidUpLeft = nullptr;
    m_pEyelidUpRight = nullptr;
    m_pEyelidDownLeft = nullptr;
    m_pEyelidDownRight = nullptr;

    m_lookTarget = nullptr;

    m_pFSM = nullptr;
}

Eyes::~Eyes()
{
}
void Eyes::SetEyesColor(const XMFLOAT3& p_color)
{
    if (p_color.x != m_baseColor.x || p_color.y != m_baseColor.y || p_color.z != m_baseColor.z)
        m_previousEyesColor = m_baseColor;

    m_baseColor = p_color;

    if (m_isAngry)
        m_materialEyes.color = cpu::ToColor(212, 13, 13);
    else if (m_hasStatusColor)
        m_materialEyes.color = m_statusColor;
    else
        m_materialEyes.color = m_baseColor;
}

void Eyes::SetStatusColor(bool active, const XMFLOAT3& color)
{
    if (active)
    {
        m_hasStatusColor = true;
        m_statusColor = color;
    }
    else if (m_hasStatusColor)
    {
        m_hasStatusColor = false;
    }
    else
    {
        return;
    }

    RefreshColor();
}

void Eyes::SetAngry(bool angry)
{
    m_isAngry = angry;
    RefreshColor();
}

void Eyes::RefreshColor()
{
    if (m_isAngry)
        m_materialEyes.color = cpu::ToColor(212, 13, 13);
    else if (m_hasStatusColor)
        m_materialEyes.color = m_statusColor;
    else
        m_materialEyes.color = m_baseColor;
}

void Eyes::SetPosition(const XMFLOAT3& p_pos)
{
    //Eyes
    m_pEyeLeft->transform.SetPosition(p_pos.x - 0.1f * m_scale, p_pos.y, p_pos.z);
    m_pEyeRight->transform.SetPosition(p_pos.x + 0.1f * m_scale, p_pos.y, p_pos.z);

    //Pupils
    m_pPupilLeft->transform.SetPosition(p_pos.x - 0.1f * m_scale, p_pos.y, p_pos.z - 0.06f * m_scale);
    m_pPupilRight->transform.SetPosition(p_pos.x + 0.1f * m_scale, p_pos.y, p_pos.z - 0.06f * m_scale);

    //Eyeslids
    m_pEyelidUpLeft->transform.SetPosition(p_pos.x - 0.1f * m_scale, p_pos.y + 0.07f * m_scale, p_pos.z - 0.1f * m_scale);
    m_pEyelidUpRight->transform.SetPosition(p_pos.x + 0.1f * m_scale, p_pos.y + 0.07f * m_scale, p_pos.z - 0.1f * m_scale);
    m_pEyelidDownLeft->transform.SetPosition(p_pos.x - 0.1f * m_scale, p_pos.y - 0.07f * m_scale, p_pos.z - 0.1f * m_scale);
    m_pEyelidDownRight->transform.SetPosition(p_pos.x + 0.1f * m_scale, p_pos.y - 0.07f * m_scale, p_pos.z - 0.1f * m_scale);
}

void Eyes::SetScale(float p_value)
{
    m_scale = p_value;

    //Eyes
    m_pEyeLeft->transform.SetScaling({ m_scale, m_scale , m_scale });
    m_pEyeRight->transform.SetScaling({ m_scale, m_scale , m_scale });

    //Pupils
    m_pPupilLeft->transform.SetScaling({ m_scale, m_scale , m_scale });
    m_pPupilRight->transform.SetScaling({ m_scale, m_scale , m_scale });

    //Eyeslids
    m_pEyelidUpLeft->transform.SetScaling({ m_scale, m_scale , m_scale });
    m_pEyelidUpRight->transform.SetScaling({ m_scale, m_scale , m_scale });
    m_pEyelidDownLeft->transform.SetScaling({ m_scale, m_scale , m_scale });
    m_pEyelidDownRight->transform.SetScaling({ m_scale, m_scale , m_scale });

    SetPosition(m_startPos);
}

XMFLOAT3 Eyes::GetPosition()
{
    XMFLOAT3 pos = m_pEyeLeft->transform.pos;
    pos.x += 1.f;
    return pos;
}

void Eyes::Create(const XMFLOAT3& p_pos)
{
    //Eyes
    m_meshEye.CreateSphere(0.06f, 5, 5, CPU_WHITE, CPU_WHITE);
    m_pEyeLeft = cpuEngine.CreateEntity();
    m_pEyeLeft->pMesh = &m_meshEye;
    m_pEyeLeft->pMaterial = &m_materialEyes;

    m_pEyeRight = cpuEngine.CreateEntity();
    m_pEyeRight->pMesh = &m_meshEye;
    m_pEyeRight->pMaterial = &m_materialEyes;

    //Pupils
    m_meshPupil.CreateSphere(0.02f, 5, 5, CPU_BLACK, CPU_BLACK);
    m_pPupilLeft = cpuEngine.CreateEntity();
    m_pPupilLeft->pMesh = &m_meshPupil;

    m_pPupilRight = cpuEngine.CreateEntity();
    m_pPupilRight->pMesh = &m_meshPupil;
    
    //Eyeslids
    m_meshEyelid.CreateBox(0.08f, 0.04f, 0.005f, CPU_BLACK);
    m_pEyelidUpLeft = cpuEngine.CreateEntity();
    m_pEyelidUpLeft->pMesh = &m_meshEyelid;

    m_pEyelidUpRight = cpuEngine.CreateEntity();
    m_pEyelidUpRight->pMesh = &m_meshEyelid;

    m_pEyelidDownLeft = cpuEngine.CreateEntity();
    m_pEyelidDownLeft->pMesh = &m_meshEyelid;

    m_pEyelidDownRight = cpuEngine.CreateEntity();
    m_pEyelidDownRight->pMesh = &m_meshEyelid;

    //FSM
    m_pFSM = cpuEngine.CreateFSM(this);
    m_pFSM->Add<StateEyesBlink>();
    m_pFSM->Add<StateEyesLook>();
    m_pFSM->Add<StateEyesIdle>();
    m_pFSM->Add<StateEyesHappy>();
    m_pFSM->Add<StateEyesAngry>();
    m_pFSM->Add<StateEyesCursorFollow>();
    m_pFSM->Add<StateEyesPuckFollow>();

    m_pFSM->ToState(2);

    m_startPos = p_pos;
    SetPosition(p_pos);
}

void Eyes::Destroy()
{
    m_pFSM = cpuEngine.Release(m_pFSM);

    m_pEyeLeft = cpuEngine.Release(m_pEyeLeft);
    m_pEyeRight = cpuEngine.Release(m_pEyeRight);

    m_pPupilLeft = cpuEngine.Release(m_pPupilLeft);
    m_pPupilRight = cpuEngine.Release(m_pPupilRight);

    m_pEyelidUpLeft = cpuEngine.Release(m_pEyelidUpLeft);
    m_pEyelidUpRight = cpuEngine.Release(m_pEyelidUpRight);
    m_pEyelidDownLeft = cpuEngine.Release(m_pEyelidDownLeft);
    m_pEyelidDownRight = cpuEngine.Release(m_pEyelidDownRight);
}

void Eyes::Move(const XMFLOAT3& p_delta)
{
    //Eyes
    XMFLOAT3 elPos = m_pEyeLeft->transform.pos;
    m_pEyeLeft->transform.SetPosition(elPos.x + p_delta.x * m_scale, elPos.y + p_delta.y * m_scale, elPos.z + p_delta.z * m_scale);
    XMFLOAT3 erPos = m_pEyeRight->transform.pos;
    m_pEyeRight->transform.SetPosition(erPos.x + p_delta.x * m_scale, erPos.y + p_delta.y * m_scale, erPos.z + p_delta.z * m_scale);

    //Pupils
    XMFLOAT3 plPos = m_pPupilLeft->transform.pos;
    m_pPupilLeft->transform.SetPosition(plPos.x + p_delta.x * m_scale, plPos.y + p_delta.y * m_scale, plPos.z + p_delta.z * m_scale);
    XMFLOAT3 prPos = m_pPupilRight->transform.pos;
    m_pPupilRight->transform.SetPosition(prPos.x + p_delta.x * m_scale, prPos.y + p_delta.y * m_scale, prPos.z + p_delta.z * m_scale);

    //Eyeslids
    XMFLOAT3 elulPos = m_pEyelidUpLeft->transform.pos;
    m_pEyelidUpLeft->transform.SetPosition(elulPos.x + p_delta.x * m_scale, elulPos.y + p_delta.y * m_scale, elulPos.z + p_delta.z * m_scale);
    XMFLOAT3 elurPos = m_pEyelidUpRight->transform.pos;
    m_pEyelidUpRight->transform.SetPosition(elurPos.x + p_delta.x * m_scale, elurPos.y + p_delta.y * m_scale, elurPos.z + p_delta.z * m_scale);
    XMFLOAT3 eldlPos = m_pEyelidDownLeft->transform.pos;
    m_pEyelidDownLeft->transform.SetPosition(eldlPos.x + p_delta.x * m_scale, eldlPos.y + p_delta.y * m_scale, eldlPos.z + p_delta.z * m_scale);
    XMFLOAT3 eldrPos = m_pEyelidDownRight->transform.pos;
    m_pEyelidDownRight->transform.SetPosition(eldrPos.x + p_delta.x * m_scale, eldrPos.y + p_delta.y * m_scale, eldrPos.z + p_delta.z * m_scale);
}

void Eyes::ChooseNextState(EyesState p_curEyesState)
{
    int randomInt = (rand() % 2) + 1;

    int nextState = (int)p_curEyesState + randomInt;

    if (nextState >= (int)EyesState::COUNT)
    {
        nextState = nextState - (int)EyesState::COUNT;
    }

    m_pFSM->ToState(nextState);
}

void Eyes::MovePupilsToward(float p_x, float p_y)
{
    float length = sqrtf(p_x * p_x + p_y * p_y);

    if (length > 1.f)
    {
        p_x /= length;
        p_y /= length;
    }

    float t = cpu::Clamp(cpuTime.delta * 10.f, 0.f, 1.f);

    AimPupil(m_pEyeLeft, m_pPupilLeft, p_x, p_y, t);
    AimPupil(m_pEyeRight, m_pPupilRight, p_x, p_y, t);
}

void Eyes::AimPupil(cpu_entity* p_pEye, cpu_entity* p_pPupil, float p_x, float p_y, float p_t)
{
    float offsetX = p_x * 0.045f * m_scale;
    float offsetY = p_y * 0.02f * m_scale;

    float radius = 0.06f * m_scale;
    float depth = radius * radius - offsetX * offsetX - offsetY * offsetY;
    float offsetZ = (depth > 0.f) ? -sqrtf(depth) : 0.f;

    XMFLOAT3 eyePos = p_pEye->transform.pos;
    XMFLOAT3& pupilPos = p_pPupil->transform.pos;

    pupilPos.x = cpu::Lerp(pupilPos.x, eyePos.x + offsetX, p_t);
    pupilPos.y = cpu::Lerp(pupilPos.y, eyePos.y + offsetY, p_t);
    pupilPos.z = cpu::Lerp(pupilPos.z, eyePos.z + offsetZ, p_t);
}

//Handle
void Eyes::HandleEyesBlink()
{
    float posY = m_startPos.y;

    float dt = cpuTime.delta;

    if (m_ratio <= 0)
        m_increase = true;
    else if (m_ratio >= 1.f)
        m_increase = false;

    if (m_increase)
        m_ratio += dt * 6.f;
    else
        m_ratio -= dt * 6.f;

    m_pEyelidUpLeft->transform.pos.y = cpu::Lerp(posY + 0.07f * m_scale, posY + 0.03f * m_scale, m_ratio);
    m_pEyelidUpRight->transform.pos.y = cpu::Lerp(posY + 0.07f * m_scale, posY + 0.03f * m_scale, m_ratio);
    m_pEyelidDownLeft->transform.pos.y = cpu::Lerp(posY - 0.07f * m_scale, posY - 0.03f * m_scale, m_ratio);
    m_pEyelidDownRight->transform.pos.y = cpu::Lerp(posY - 0.07f * m_scale, posY - 0.03f * m_scale, m_ratio);
}

void Eyes::HandleEyesLook(float p_totalTime)
{
    XMFLOAT3 pos = m_startPos;

    float dt = cpuTime.delta;

    if (m_ratio <= 0)
        m_increase = true;
    else if (m_ratio >= 1.f)
        m_increase = false;

    if (m_increase)
        m_ratio += dt * 1.5f;
    else
        m_ratio -= dt * 1.5f;

    if (p_totalTime < 1.35f)
    {
        float ratio2 = std::abs(- 1.f + (m_ratio * 2.f));

        m_pPupilLeft->transform.pos.x = cpu::Lerp(pos.x - 0.04f * m_scale, pos.x - 0.16f * m_scale, m_ratio);
        m_pPupilLeft->transform.pos.y = cpu::Lerp(pos.y - 0.03f * m_scale, pos.y, ratio2);

        m_pPupilRight->transform.pos.x = cpu::Lerp(pos.x + 0.16f * m_scale, pos.x + 0.04f * m_scale, m_ratio);
        m_pPupilRight->transform.pos.y = cpu::Lerp(pos.y - 0.03f * m_scale, pos.y, ratio2);
    }
    else
    {

        if (m_pPupilLeft->transform.pos.y >= pos.y)
            return;

        m_pPupilLeft->transform.pos.y = cpu::Lerp(pos.y - 0.03f * m_scale, pos.y + 0.02f * m_scale, m_ratio);

        m_pPupilRight->transform.pos.y = cpu::Lerp(pos.y - 0.03f * m_scale, pos.y + 0.02f * m_scale, m_ratio);
    }
}

void Eyes::HandleEyesHappy(float p_totalTime)
{
    float posY = m_startPos.y;

    float dt = cpuTime.delta;

    if (m_ratio <= 0)
        m_increase = true;
    else if (m_ratio >= 1.f)
        m_increase = false;

    if (m_increase)
        m_ratio += dt * 1.5f;
    else
        m_ratio -= dt * 1.5f;

    if (p_totalTime < 0.3f)
    {
        m_pEyelidDownLeft->transform.pos.y = cpu::Lerp(posY - 0.07f * m_scale, posY - 0.04f * m_scale, m_ratio);
        m_pEyelidDownRight->transform.pos.y = cpu::Lerp(posY - 0.07f * m_scale, posY - 0.04f * m_scale, m_ratio);

        m_pEyelidUpLeft->transform.pos.y = cpu::Lerp(posY + 0.07f * m_scale, posY + 0.05f * m_scale, m_ratio);
        m_pEyelidUpRight->transform.pos.y = cpu::Lerp(posY + 0.07f * m_scale, posY + 0.05f * m_scale, m_ratio);

    }
    else if (p_totalTime >= 0.3f && p_totalTime < 1.4f)
    {
        Move({ 0, m_inv * dt / 2.7f, 0 });

        if (GetPosition().y < posY - 0.02f * m_scale || GetPosition().y > posY + 0.02f * m_scale)
            m_inv *= -1;

        if (p_totalTime > 1.37f)
        {
            m_ratio = 1.f;
            SetPosition(m_startPos);
        }
    }
    else
    {
        m_pEyelidDownLeft->transform.pos.y = cpu::Lerp(posY - 0.04f * m_scale, posY - 0.07f * m_scale, m_ratio);
        m_pEyelidDownRight->transform.pos.y = cpu::Lerp(posY - 0.04f * m_scale, posY - 0.07f * m_scale, m_ratio);

        m_pEyelidUpLeft->transform.pos.y = cpu::Lerp(posY + 0.05f * m_scale, posY + 0.07f * m_scale, m_ratio);
        m_pEyelidUpRight->transform.pos.y = cpu::Lerp(posY + 0.05f * m_scale, posY + 0.07f * m_scale, m_ratio);
    }
}

void Eyes::HandleEyesAngry(float p_totalTime)
{
    float posY = m_startPos.y;

    float dt = cpuTime.delta;

    if (m_ratio <= 0)
        m_increase = true;
    else if (m_ratio >= 1.f)
        m_increase = false;

    if (m_increase)
        m_ratio += dt * 1.5f;
    else
        m_ratio -= dt * 1.5f;

    if (p_totalTime < 0.3f)
    {
        m_pEyelidUpLeft->transform.pos.y = cpu::Lerp(posY + 0.06f * m_scale, posY + 0.04f * m_scale, m_ratio);
        m_pEyelidUpRight->transform.pos.y = cpu::Lerp(posY + 0.06f * m_scale, posY + 0.04f * m_scale, m_ratio);

        m_eyelidUpAngle += dt;

        m_pEyelidUpLeft->transform.SetYPR(0, 0, -m_eyelidUpAngle);
        m_pEyelidUpRight->transform.SetYPR(0, 0, m_eyelidUpAngle);
    }
    else if(p_totalTime >= 0.3f && p_totalTime < 1.7f)
    {
        Move({ m_d * dt * 20.f, 0, 0 });
        m_d *= -1.f;
    }
    else
    {
        m_pEyelidUpLeft->transform.pos.y = cpu::Lerp(posY + 0.04f * m_scale, posY + 0.06f * m_scale, m_ratio);
        m_pEyelidUpRight->transform.pos.y = cpu::Lerp(posY + 0.04f * m_scale, posY + 0.06f * m_scale, m_ratio);

        m_eyelidUpAngle -= dt;

        m_pEyelidUpLeft->transform.SetYPR(0, 0, -m_eyelidUpAngle);
        m_pEyelidUpRight->transform.SetYPR(0, 0, m_eyelidUpAngle);
    }
}

void Eyes::HandleEyesCursorFollow(float p_totalTime)
{
    float gazeX = 0.f;
    float gazeY = 0.f;

    XMFLOAT2 cursor;
    cpuEngine.GetCursor(cursor);

    float halfWidth = cpuDevice.GetWidth() * 0.5f;
    float halfHeight = (cpuDevice.GetHeight() * 0.5f) - cpuDevice.GetHeight() * 0.25f;

    const float invHalfWidth = 1.f / halfWidth;
    const float invHalfHeight = 1.f / halfHeight;

    gazeX = (cursor.x - halfWidth) * invHalfWidth;
    gazeY = -(cursor.y - halfHeight) * invHalfHeight;  

    MovePupilsToward(gazeX, gazeY);
}

void Eyes::HandleEyesPuckFollow(float p_totalTime)
{
    float gazeX = 0.f;
    float gazeY = 0.f;

    float dx = m_lookTarget->transform.pos.x - m_startPos.x;
    float dy = 0.14f - m_startPos.y;
    float dz = m_lookTarget->transform.pos.z - m_startPos.z;

    float yaw = atan2f(dx, -dz);
    float pitch = atan2f(dy, sqrtf(dx * dx + dz * dz));

    gazeX = yaw * (180.f / (XM_PI * 30.f));
    gazeY = pitch * (180.f / (XM_PI * 20.f));

    MovePupilsToward(gazeX, gazeY);
}


//StateMachine
void StateEyesBlink::OnEnter(Eyes& cur, int from, void* pParam)
{
    cur.SetRatio(0);
}

void StateEyesBlink::OnExecute(Eyes& cur)
{
    cur.HandleEyesBlink();
    if (cur.GetFSM()->totalTime >= 0.50f && cur.GetRatio() < 0.3f)
    {
        cur.GetFSM()->ToState(2);
        return;
    }
}

void StateEyesBlink::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesLook::OnEnter(Eyes& cur, int from, void* pParam)
{
    cur.SetRatio(0.5f);
}

void StateEyesLook::OnExecute(Eyes& cur)
{
    cur.HandleEyesLook(cur.GetFSM()->totalTime);

    if (cur.GetFSM()->totalTime >= 1.4f)
    {       
        cur.GetFSM()->ToState(2);
        return;        
    }
}

void StateEyesLook::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesIdle::OnEnter(Eyes& cur, int from, void* pParam)
{
}

void StateEyesIdle::OnExecute(Eyes& cur)
{
    if (cur.GetFSM()->totalTime > 3.0f)
    {
        cur.ChooseNextState(EyesState::IDLE);
        return;
    } 
}

void StateEyesIdle::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesHappy::OnEnter(Eyes& cur, int from, void* pParam)
{
}

void StateEyesHappy::OnExecute(Eyes& cur)
{
    cur.HandleEyesHappy(cur.GetFSM()->totalTime);

    if (cur.GetFSM()->totalTime > 1.6f)
    {
        cur.GetFSM()->ToState(2);
        return;
    }
}

void StateEyesHappy::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesAngry::OnEnter(Eyes& cur, int from, void* pParam)
{
    cur.SetAngry(true);
}

void StateEyesAngry::OnExecute(Eyes& cur)
{
    cur.HandleEyesAngry(cur.GetFSM()->totalTime);

    if (cur.GetFSM()->totalTime > 2.0f)
    {
        cur.GetFSM()->ToState(2);
        return;
    }
}

void StateEyesAngry::OnExit(Eyes& cur, int to)
{
    cur.SetAngry(false);
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesCursorFollow::OnEnter(Eyes& cur, int from, void* pParam)
{
}

void StateEyesCursorFollow::OnExecute(Eyes& cur)
{
    cur.HandleEyesCursorFollow(cur.GetFSM()->totalTime);
}

void StateEyesCursorFollow::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}


void StateEyesPuckFollow::OnEnter(Eyes& cur, int from, void* pParam)
{
}

void StateEyesPuckFollow::OnExecute(Eyes& cur)
{
    cur.HandleEyesPuckFollow(cur.GetFSM()->totalTime);
}

void StateEyesPuckFollow::OnExit(Eyes& cur, int to)
{
    cur.SetPosition(cur.GetStartPosition());
}
