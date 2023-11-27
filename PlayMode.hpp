#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <deque>
#include <list>
#include "glm/gtx/string_cast.hpp"

struct PlayMode : Mode
{
	PlayMode();
	virtual ~PlayMode();

	// functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	glm::vec3 bullet_current_Pos(glm::vec3 origin_Pos, glm::vec3 final_Pos, float time);
	void hit_player();
	void hit_boss();
	bool on_platform();
	bool hit_platform();
	void land_on_platform(glm::vec3 expected_position);
	//----- game state -----

	// input tracking:
	struct Button
	{
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} keya, keyd, keys, keyw, keyatk, space;

	// local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// glm::vec3 get_leg_tip_position();
	Scene::Transform *player = nullptr;
	// player attack once
	bool attack = false;
	Scene::Transform *player_atk_cpnt;
	bool get_weapon = false;
	// component
	Scene::Transform *component = nullptr;
	glm::vec3 component_scale;

	// cage
	struct Cage
	{
		int index = 0;
		Scene::Transform *transform = nullptr;
		bool isDestroied = false;
	};
	int cage_index = 0;
	std::vector<Cage> cages;

	// boots
	Scene::Transform *boots = nullptr;
	bool hasBoots = false;
	Scene::Transform *component_boots = nullptr;
	glm::vec3 boots_scale;

	bool first_jump;
	bool second_jump;

	float jump_velocity = 0.0f;
	bool jump_signal = false;

	// jet pack
	Scene::Transform *jetPack = nullptr;
	bool hasJetPack = false;
	bool jetpack_on = false;
	Scene::Transform *component_jetpack = nullptr;
	glm::vec3 jetpack_scale;
	float jetpack_max_fuel = 1.5f;
	float jetpack_fuel = 0.0f;
	float jetpack_max_speed = 4.5f;
	float hover_full_fuel_time = 0.1f;
	float hover_max_time = 0.4f;
	float hover_time = 0.0f;

	glm::vec3 start_point;
	glm::vec3 player_origin_scale;
	bool face_right = true;

	// boss weapon
	Scene::Transform *boss_weapon = nullptr;
	int weapon_timer = 0;
	// bullet
	typedef struct Bullet
	{
		int index = 0;
		Scene::Transform *transform = nullptr;

		glm::vec3 original_pos = glm::vec3(0, 0, 0);
		glm::vec3 final_pos = glm::vec3(0, 0, 0);

		glm::vec3 player_pos = glm::vec3(0, 0, 0);

		float bullet_current_time = 0;
		bool hit_player = false;
	} Bullet;

	std::vector<Bullet> bullets;
	Bullet current_bullet;

	float bullet_total_time = 0;
	std::list<Bullet> current_bullets; // each three bullets in a group
	int current_bullets_index = 0;	   // the index in the current_bullets
	int bullet_index = 0;			   // all bullet index
	int bullet_current_index = 0;	   // put next index
	int bullet_count = 3;
	float bullet_speed = 3.0f;

	bool shooting1 = true;
	bool shooting2 = true;
	bool shooting3 = true;
	bool hit1 = false;
	bool hit2 = false;
	bool hit3 = false;

	struct SubUV
	{
		std::vector<Scene::Transform *> subtransforms;
		uint32_t bitmask = 1;
		uint32_t start_index = 0;
		uint32_t range = 0;
		float anim_timer = 0.0f;
		float speed = 1.0f;
		float scale = 1.0f;
	};

	// player status
	enum class PlayerStatus
	{
		Idle,
		MoveLeft,
		MoveRight,
		JumpStart,
		JumpLoop,
		JumpEnd,
		Hurt,
		Dead
	};

	enum class PlayerStage
	{
		InitialStage,
		JumpGame,
		BossTeleport
	};

	PlayerStage player_stage = PlayerStage::InitialStage;

	SubUV subuv;
	PlayerStatus player_status = PlayerStatus::Idle;
	bool player_die = false;

	// boss status
	enum BattleStatus
	{
		Melee,
		Shoot,
		Idle
	};
	bool finish_bullet = false;
	BattleStatus boss_status = Shoot;

	/// boss
	Scene::Transform *boss = nullptr;
	float boss_speed = 1.0f;
	bool boss_die = false;

	/// bosshp
	Scene::Transform *boss_hp = nullptr;
	float max_boss_hp = 1.0f;
	float current_boss_hp = 1.0f;

	/// playerhp
	Scene::Transform *player_hp = nullptr;
	float max_player_hp = 1.0f;
	float current_player_hp = 1.0f;

	std::list<Scene::Transform *> outerList;

	typedef struct Platform
	{
		std::string name = "";
		glm::vec3 pos = glm::vec3(0, 0, 0);
		float height = 0;
		float width = 0;
	} Platform;

	std::list<Platform> platforms;

	// hit object
	typedef struct HitObject
	{
		std::string name;
		bool overlapped = false;
	} HitObject;
	// hit_obj
	HitObject hit_detect_obj;

	// music coming from the tip of the leg (as a demonstration):
	std::shared_ptr<Sound::PlayingSample> leg_tip_loop;

	// camera:
	Scene::Camera *camera = nullptr;

	void put_away_bullet(Bullet bullet);

	void update_player_status();

	HitObject hit_detect(Scene::Transform *obj, Scene::Transform *hit_obj);
	HitObject hit_detect_SAT(Scene::Transform *obj, Scene::Transform *hit_obj);
	std::pair<float,float> ProjectAlongVector(Scene::Transform *obj, const glm::vec3& projectionVector);
};
