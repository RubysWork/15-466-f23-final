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

	//----- game state -----

	// input tracking:
	struct Button
	{
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} keya, keyd, keys, keyw, space;

	// local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// glm::vec3 get_leg_tip_position();
	Scene::Transform *player = nullptr;

	glm::vec3 start_point;

	bool first_jump;
	bool second_jump;
	
	float jump_velocity = 0.0f;
	bool jump_signal = false;

	/// boss
	Scene::Transform *boss = nullptr;

	typedef struct Bullet
	{
		int index = 0;
		Scene::Transform *transform = nullptr;
		glm::vec3 shoot_orient = glm::vec3(0, 0, 0);
	} Bullet;

	std::list<Bullet> bullets;
	int bullet_index = 0;

	enum BattleStatus
	{
		Melee,
		Shoot
	};

	BattleStatus boss_status = Shoot;
	int bullet_count = 3;
	///

	// music coming from the tip of the leg (as a demonstration):
	std::shared_ptr<Sound::PlayingSample> leg_tip_loop;

	// camera:
	Scene::Camera *camera = nullptr;
};
