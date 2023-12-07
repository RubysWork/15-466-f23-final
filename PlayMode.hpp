#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "DrawText.hpp"

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
	glm::vec3 check_change_stage(glm::vec3 expected_position);
	glm::vec3 bullet_current_Pos(glm::vec3 origin_Pos, glm::vec3 final_Pos, float time);
	void hit_player(float damage); // damage max=1
	void hit_spike();
	void hit_boss(float damage); // damage max=1
	bool on_platform(Scene::Transform *obj, Scene::Transform &stepped_platform);
	void on_platform_step(float elapsed);
	bool hit_platform();
	void land_on_platform(glm::vec3 expected_position);
	void check_dropping();
	void revive(float elapsed);
	glm::vec3 enemy_land_on_platform(Scene::Transform *enemy, glm::vec3 expected_position);
	void change_rand_pos();

	//----- game state -----

	// input tracking:
	struct Button
	{
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} keya, keyd, keys, keyw, keyatk, space, enter;

	// text
	DrawText text;
	GLuint text_program;

	// local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	Scene::Transform *start_menu = nullptr;

	Scene::Transform *player = nullptr;
	// player attack once
	bool attack = false;
	Scene::Transform *player_atk_cpnt;
	bool get_weapon = false;
	// component
	Scene::Transform *component = nullptr;
	glm::vec3 component_scale;

	// boots+star
	struct Item
	{
		Scene::Transform *transform = nullptr;
		bool has = false;
	};
	Item boots;

	int star_idx = 1;
	int star_count = 0;

	// cage
	struct Cage
	{
		int index = 0;
		Scene::Transform *transform = nullptr;
		bool isDestroied = false;
		Item *item = nullptr;
	};
	int cage_index = 0;
	std::vector<Cage> cages;

	// boots
	// Scene::Transform *boots = nullptr;
	// bool hasBoots = false;
	Scene::Transform *component_boots = nullptr;
	glm::vec3 boots_scale;
	float boots_timer = 0.0f;

	// wings
	Scene::Transform *component_wings = nullptr;
	glm::vec3 wings_scale;
	float wings_timer = 0.0f;

	bool first_jump;
	bool second_jump;

	float jump_velocity = 0.0f;
	bool jump_signal = false;

	float max_fall_speed = 0;

	// jet pack
	Scene::Transform *jetPack = nullptr;
	bool hasJetPack = false;
	bool jetpack_on = false;
	Scene::Transform *component_jetpack = nullptr;
	glm::vec3 jetpack_scale;
	float jetpack_max_fuel = 1.5f;
	float jetpack_fuel = 0.0f;
	float jetpack_max_speed = 1.625f;
	// float hover_full_fuel_time = 0.1f;
	// float hover_max_time = 0.4f;
	// float hover_time = 0.0f;

	// wings
	Scene::Transform *wings = nullptr;
	bool hasWings = false;
	bool beatWings = false;
	bool flying = false;
	float wings_max_energy = 15.0f;
	float wings_energy = 15.0f;

	Scene::Transform *player_fuel = nullptr;
	float max_fuel_scale = 0.0f;

	glm::vec3 start_point;
	glm::vec3 player_origin_scale;
	bool face_right = true;

	struct SubUV
	{
		std::vector<Scene::Transform *> subtransforms;
		uint64_t bitmask = 1;
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

	enum class WeaponStatus
	{
		Idle,
		NormalAttack
	};

	enum class PlayerStage
	{
		InitialStage,
		JumpGame,
		BossTeleport
	};

	PlayerStage player_stage = PlayerStage::InitialStage;
	float stage_changing = false;
	float stage_change_timer = 0.0f;
	int death_time = 0;

	SubUV subuv;
	SubUV weapon_subuv;
	SubUV wings_subuv;
	SubUV boss_subuv;
	SubUV final_subuv;
	std::vector<SubUV> explode_subuv;
	int explode_subuv_count = 0;
	std::vector<SubUV> enemy_subuv;
	int enemy_subuv_count = 0;
	PlayerStatus player_status = PlayerStatus::Idle;
	WeaponStatus weapon_status = WeaponStatus::Idle;
	bool player_die = false;

	// boss status
	enum BattleStatus
	{
		Melee,
		Shoot,
		Idle,
		Attacked,
		Weak, // boss will stop attack, hide weapon
		Dead
	};
	bool finish_bullet = false;
	BattleStatus boss_status = Shoot;
	bool detect_boss_status = true; // if false, boss status won't change
	float attacked_timer = 0.0f;

	bool start_weak_timer = false;
	int weak_timer = 0; // idle(hide boss weapon) every 3s

	// boss weapon
	typedef struct BossWeapon
	{
		Scene::Transform *transform = nullptr;
		glm::vec3 ori_weap_scale = glm::vec3(0);
		int timer = 0;
	} BossWeapon;

	// BossWeapon *current_boss_weapon;
	BossWeapon level1_boss_weapon;
	BossWeapon final_boss_weapon;

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

	bool shooting1 = true; // first bullet is shooting
	bool shooting2 = true;
	bool shooting3 = true;
	bool hit1 = false;
	bool hit2 = false;
	bool hit3 = false;

	/// boss
	typedef struct Boss
	{
		Scene::Transform *transform;
		float speed = 1.0f;
		float max_hp = 1.0f;
		float current_hp = 1.0f;
		bool die = false;
		bool hasweapon = true;
		BossWeapon *weapon;
		BattleStatus status = Idle;
	} Boss;

	Boss *current_boss;
	Boss level1_boss;
	Boss final_boss;

	/// bosshp
	Scene::Transform *boss_hp = nullptr;
	glm::vec3 ori_bosshp_scale = glm::vec3(1);
	Scene::Transform *boss_hp_bg = nullptr; // hide hp

	// boss teleport
	std::list<glm::vec3> final_teleportPos;
	Scene::Transform *level1_tel;
	int count_for_teleport = 0; // player hit boss, if this count reaches 3, start teleport
	bool ready_to_teleport = false;
	float teleport_timer = 0.3f; // scale cange timer
	bool arrive_new_pos = false; // start scale

	// final boss random move
	std::list<glm::vec3> final_move_pos;
	glm::vec3 rand_pos;
	int rand_move_timer = 0; // start rand move
	float rand_pos_time = 0; // current rand move time
	bool first_melee = false;

	// place boom
	struct Boom
	{
		int index = 0;
		Scene::Transform *transform = nullptr;
		Scene::Transform *explode = nullptr;
		bool ready_explode = false; // replace boom by explode
		bool start_explode = false; // play explode ani
		float explode_countdown = 0;
		glm::vec3 ori_scale = glm::vec3(0);
	};
	std::list<Boom> booms;
	int boom_count = 0;
	int last_boom_idx = 0;
	int explode_idx = 0;
	float place_boom_timer = 0;

	// enemy_gravity
	float enemy_gravity = -0.5f;

	// enemy status
	enum class EnemyStatus
	{
		Idle,
		Move,
		Damaged,
		Dead
	};

	typedef struct Enemy
	{
		int index = 0;
		Scene::Transform *transform;
		float speed = 1.0f;
		float max_hp = 1.0f;
		float current_hp = 1.0f;
		EnemyStatus status = EnemyStatus::Idle;
		Scene::Transform *stepped_plat;
		bool canmove = true; // static enemy or moving enemy
		float damage_time = 0.0f;
	} Enemy;

	std::list<Enemy> enemies;
	Enemy *current_enemy;

	/// playerhp
	Scene::Transform *player_hp = nullptr;
	float max_player_hp = 1.0f;
	float current_player_hp = 1.0f;

	float invincible_time = 0.3f;
	bool invincible = false;

	float revive_time = 3.25f;
	float revive_max_time = 3.25f;
	float waiting_revive = false;

	std::list<Scene::Transform *> outerList;
	typedef struct Spike
	{
		glm::vec3 pos = glm::vec3(0, 0, 0);
		float height = 0;
		float width = 0;
	} Spike;

	std::list<Spike> spikes;

	typedef struct Platform
	{
		Scene::Transform *transform = nullptr;
		std::string name = "";
		glm::vec3 pos = glm::vec3(0, 0, 0);
		float height = 0;
		float width = 0;
		bool fragile = false;
		bool visible = true;
		float stepping_time = 0.0f;
	} Platform;

	std::list<Platform> platforms;
	std::list<Platform> fragiles;

	// hit object
	typedef struct HitObject
	{
		std::string name;
		bool overlapped = false;
	} HitObject;
	// hit_obj
	HitObject hit_detect_obj;

	// music coming from the tip of the leg (as a demonstration):
	Sound::Listener listener;
	std::shared_ptr<Sound::PlayingSample> music;
	std::shared_ptr<Sound::PlayingSample> sound;
	std::shared_ptr<Sound::PlayingSample> wings_sound;
	std::shared_ptr<Sound::PlayingSample> boss1_loop_sound;
	std::shared_ptr<Sound::PlayingSample> fuse_sound;
	std::shared_ptr<Sound::PlayingSample> explode_sound;

	// camera:
	Scene::Camera *camera = nullptr;

	bool game_end = false;

	void put_away_bullet(Bullet bullet);

	void update_player_status();
	void update_weapon_status();
	void update_wings_status();

	glm::vec3 nearest_teleport();

	void update_boss_status();
	bool level1_boss_dead();
	void update_enemy_status(Enemy enemy);
	void enemy_dead(Enemy enemy);
	void update_final_status();
	bool final_boss_dead();

	void teleport();
	void play_explode_ani(Boom *boom);

	HitObject hit_detect(Scene::Transform *obj, Scene::Transform *hit_obj);
	HitObject hit_detect_SAT(Scene::Transform *obj, Scene::Transform *hit_obj);
	std::pair<float, float> ProjectAlongVector(Scene::Transform *obj, const glm::vec3 &projectionVector);

	// test_platform, now used for enemy default platform
	Scene::Transform *fragile5 = nullptr;
};
