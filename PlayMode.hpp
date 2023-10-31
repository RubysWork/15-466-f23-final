#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>
#include <deque>

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

	glm::vec3 start_point;

	bool first_jump;
	bool second_jump;

	float jump_velocity = 0.0f;
	bool jump_signal = false;

	glm::vec3 player_origin_scale;
	bool face_right = true;

	// bullet
	typedef struct Bullet
	{
		int index = 0;
		Scene::Transform *transform = nullptr;

		glm::vec3 original_pos = glm::vec3(0, 0, 0);
		glm::vec3 final_pos = glm::vec3(0, 0, 0);

		glm::vec3 player_pos = glm::vec3(0, 0, 0);
	} Bullet;

	std::vector<Bullet> bullets;

	std::list<Scene::Transform *> outerList;

	typedef struct Platform
	{
		glm::vec3 pos = glm::vec3(0, 0, 0);
		float height = 0;
		float width = 0;
	} Platform;

	std::list<Platform> platforms;

	Bullet current_bullet;
	int bullet_index = 0;
	int bullet_current_index = 0;
	int bullet_count = 3;
	float bullet_speed = 1.5f;
	float bullet_current_time = 0;
	int one_bullet_timer = 0;

	// boss status
	enum BattleStatus
	{
		Melee,
		Shoot
	};
	BattleStatus boss_status = Shoot;
	/// boss
	Scene::Transform *boss = nullptr;
	// player attack once
	bool attack = false;
	/// bosshp
	Scene::Transform *boss_hp = nullptr;
	float max_boss_hp = 1.0f;
	float current_boss_hp = 1.0f;
	/// playerhp
	Scene::Transform *player_hp = nullptr;
	float max_player_hp = 1.0f;
	float current_player_hp = 1.0f;

	// component
	Scene::Transform *component = nullptr;

	// music coming from the tip of the leg (as a demonstration):
	std::shared_ptr<Sound::PlayingSample> leg_tip_loop;

	// camera:
	Scene::Camera *camera = nullptr;

	void put_away_bullet(Bullet bullet);
};
