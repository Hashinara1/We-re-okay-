#pragma once

enum class EyesState
{
	BLINK,
	LOOK,
	IDLE,

	COUNT
};

class Eyes
{
public:
	Eyes();
	~Eyes();

	void SetEyesColor(const XMFLOAT3& p_color);
	void SetStatusColor(bool active, const XMFLOAT3& color);
	void SetAngry(bool angry);
	void RefreshColor();
	void SetPosition(const XMFLOAT3& p_pos);
	void SetRatio(float p_ratio) { m_ratio = cpu::Clamp(p_ratio, 0.f, 1.f); }
	void SetScale(float p_value); 
	void SetLookTarget(cpu_entity* p_lookTarget) { m_lookTarget = p_lookTarget; }

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetStartPosition() { return m_startPos; };
	XMFLOAT3 GetPreviousEyesColor() { return m_previousEyesColor; }
	float GetRatio() { return m_ratio; }

	void Create(const XMFLOAT3& p_pos);
	
	void Destroy();

	void Move(const XMFLOAT3& p_delta);
	void MovePupilsToward(float p_x, float p_y);

	void ChooseNextState(EyesState p_curEyesState);
	void AimPupil(cpu_entity* p_pEye, cpu_entity* p_pPupil, float p_x, float p_y, float p_t);

	void HandleEyesBlink();
	void HandleEyesLook(float p_totalTime);
	void HandleEyesHappy(float p_totalTime);
	void HandleEyesAngry(float p_totalTime);
	void HandleEyesCursorFollow(float p_totalTime);
	void HandleEyesPuckFollow(float p_totalTime);

	cpu_fsm<Eyes>* GetFSM() { return m_pFSM; }

protected:

	cpu_fsm<Eyes>* m_pFSM;

private:

// Mesh
	cpu_mesh m_meshEye;
	cpu_mesh m_meshPupil;
	cpu_mesh m_meshEyelid;

//Material
	cpu_material m_materialEyes;

// Entity
	//Eyes
	cpu_entity* m_pEyeLeft;
	cpu_entity* m_pEyeRight;
	//Pupils
	cpu_entity* m_pPupilLeft;
	cpu_entity* m_pPupilRight;
	//Eyelid UP
	cpu_entity* m_pEyelidUpLeft;
	cpu_entity* m_pEyelidUpRight;
	//Eyelid Down
	cpu_entity* m_pEyelidDownLeft;
	cpu_entity* m_pEyelidDownRight;

	//LookTarget
	cpu_entity* m_lookTarget;	

	XMFLOAT3 m_startPos = { 0.f, 0.f, 0.f };

	XMFLOAT3 m_baseColor = CPU_WHITE;
	XMFLOAT3 m_previousEyesColor = CPU_WHITE;
	XMFLOAT3 m_statusColor = CPU_WHITE;
	bool m_hasStatusColor = false;
	bool m_isAngry = false;

	float m_scale = 1.f;

	bool m_increase = true;

	float m_ratio = 0.f;
	float m_d = 0.02f;
	float m_eyelidUpAngle = 0.f;

	int m_inv = 1;
};

struct StateEyesBlink 
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesLook
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesIdle
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesHappy
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesAngry
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesCursorFollow
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};

struct StateEyesPuckFollow
{
	void OnEnter(Eyes& cur, int from, void* pParam);
	void OnExecute(Eyes& cur);
	void OnExit(Eyes& cur, int to);
};