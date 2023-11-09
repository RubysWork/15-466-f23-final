#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <deque>
#include <list>

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

	glm::vec3 start_point;

	bool first_jump;
	bool second_jump;

	float jump_velocity = 0.0f;
	bool jump_signal = false;

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
		glm::vec3 pos = glm::vec3(0, 0, 0);
		float height = 0;
		float width = 0;
	} Platform;

	std::list<Platform> platforms;

	// music coming from the tip of the leg (as a demonstration):
	std::shared_ptr<Sound::PlayingSample> leg_tip_loop;

	// camera:
	Scene::Camera *camera = nullptr;

	void put_away_bullet(Bullet bullet);
};
