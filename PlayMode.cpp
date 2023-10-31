#include "PlayMode.hpp"

#include "MyLitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> meshes(LoadTagDefault, []() -> MeshBuffer const *
						{
	MeshBuffer const *ret = new MeshBuffer(data_path("cube.pnct"));
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(my_lit_color_texture_program->program);
	return ret; });

Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const *
						  { return new Scene(
								data_path("cube.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name)
								{
									Mesh const mesh = meshes->lookup(mesh_name);

									scene.drawables.emplace_back(transform);
									Scene::Drawable &drawable = scene.drawables.back();
									drawable.pipeline = my_lit_color_texture_program_pipeline;
									drawable.pipeline.type = mesh.type;
									drawable.pipeline.start = mesh.start;
									drawable.pipeline.count = mesh.count;

									drawable.pipeline.vao = meshes_for_lit_color_texture_program; }); });

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const *
									   { return new Sound::Sample(data_path("dusty-floor.opus")); });

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Player")
		{
			player = &transform;
			start_point = player->position;
		}

		else if (transform.name == "Boss")
		{
			boss = &transform;
		}

		else if (transform.name.find("Bullet") != std::string::npos)
		{
			Bullet bullet;
			bullet.index = bullet_index;
			bullet.transform = &transform;
			bullet.transform->scale = glm::vec3(0, 0, 0);
			bullet.original_pos = bullet.transform->position;

			bullets.emplace_back(bullet);

			bullet_index++;
		}
		else if (transform.name == "BossHp")
		{
			boss_hp = &transform;
		}
		else if (transform.name == "PlayerHp")
		{

			player_hp = &transform;
		}
	}
	for (auto &bullet : bullets)
	{
		bullet.player_pos = player->position;
	}

	// get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// start music loop playing:
	//  (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a)
		{
			keya.downs += 1;
			keya.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			keyd.downs += 1;
			keyd.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			keyw.downs += 1;
			keyw.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			keys.downs += 1;
			keys.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			keya.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			keyd.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			keyw.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			keys.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.pressed = false;
			return true;
		}
	}
	// else if (evt.type == SDL_MOUSEBUTTONDOWN)
	// {
	// 	if (SDL_GetRelativeMouseMode() == SDL_FALSE)
	// 	{
	// 		SDL_SetRelativeMouseMode(SDL_TRUE);
	// 		return true;
	// 	}
	// }
	// else if (evt.type == SDL_MOUSEMOTION)
	// {
	// 	if (SDL_GetRelativeMouseMode() == SDL_TRUE)
	// 	{
	// 		glm::vec2 motion = glm::vec2(
	// 			evt.motion.xrel / float(window_size.y),
	// 			-evt.motion.yrel / float(window_size.y));
	// 		camera->transform->rotation = glm::normalize(
	// 			camera->transform->rotation * glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f)));
	// 		return true;
	// 	}
	// }

	return false;
}

void PlayMode::update(float elapsed)
{
	if (player_hp->scale.x <= 0.0001f)
	{
		player->scale = glm::vec3(0);
	}

	switch (boss_status)
	{
	case Melee:

		break;
	case Shoot:

		bullet_current_time += bullet_speed * elapsed;
		current_bullet = *(bullets.begin() + bullet_current_index);
		current_bullet.transform->scale = glm::vec3(0.15f);
		current_bullet.transform->position.y = player->position.y;
		current_bullet.transform->position = bullet_current_Pos(boss->position, current_bullet.player_pos, bullet_current_time);

		one_bullet_timer++;
		// std::cout << one_bullet_timer << std::endl;
		// generate new one
		if (current_bullet.transform->position.x < player->position.x + 0.4f && current_bullet.transform->position.x > player->position.x - 0.4f && current_bullet.transform->position.z < player->position.z + 0.4f && current_bullet.transform->position.z > player->position.z - 0.4f)
		{
			put_away_bullet(current_bullet);
			hit_player();
		}
		if (one_bullet_timer > 1000)
		{
			put_away_bullet(current_bullet);
		}

		break;
	default:

		break;
	}

	// move camera:
	{
		// combine inputs into a move:
		constexpr float PlayerSpeed = 2.0f;
		// move left and right
		float move = 0.0f;
		if (keya.pressed && !keyd.pressed)
			move = -1.0f;
		if (!keya.pressed && keyd.pressed)
			move = 1.0f;

		if (space.pressed)
		{
			if (!first_jump)
			{
				jump_signal = false;
				first_jump = true;
				jump_velocity = 3.0f;
			}
			else if (first_jump && !second_jump && jump_signal)
			{
				jump_signal = false;
				second_jump = true;
				jump_velocity = 3.0f;
			}
			else
			{
				// do nothing
			}
		}

		if (!space.pressed)
		{
			jump_signal = true;
		}

		float gravity = -4.0f;

		// if (down.pressed && !up.pressed)
		// 	move.y = -1.0f;
		// if (!down.pressed && up.pressed)
		// 	move.y = 1.0f;

		// make it so that moving diagonally doesn't go faster:
		float vert_move = move * PlayerSpeed * elapsed;

		jump_velocity += gravity * elapsed;
		float hori_move = jump_velocity * elapsed;

		// std::cout << first_jump << ", " << second_jump << ", " << jump_interval << "\n";
		// std::cout << jump_velocity << "\n";

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_up = frame[1];
		// glm::vec3 frame_forward = -frame[2];

		if (player->position.z == start_point.z)
		{
			first_jump = false;
			second_jump = false;
			jump_velocity = 0;
			jump_signal = false;
		}

		glm::vec3 expected_position = player->position + vert_move * frame_right + hori_move * frame_up;
		if (expected_position.z < start_point.z)
		{
			expected_position.z = start_point.z;
		}

		// camera->transform->position += move.x * frame_right + move.y * frame_forward;
		// player->position += move.x * frame_right + move.y * frame_forward + move.z * frame_up;
		camera->transform->position += expected_position - player->position;
		player->position = expected_position;
	}

	{ // update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	// reset button press counters:
	keya.downs = 0;
	keyd.downs = 0;
	keyw.downs = 0;
	keys.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	// update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	// set up light type and position for lit_color_texture_program:
	//  TODO: consider using the Light(s) in the scene to do this
	glUseProgram(my_lit_color_texture_program->program);
	glUniform1i(my_lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(my_lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(my_lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));

	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ // use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
						glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

// glm::vec3 PlayMode::get_leg_tip_position()
// {
// 	// the vertex position here was read from the model in blender:
// 	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
// }
glm::vec3 PlayMode::bullet_current_Pos(glm::vec3 origin_Pos, glm::vec3 final_Pos, float time)
{
	glm::vec3 dir = glm::normalize(final_Pos - origin_Pos);
	glm::vec3 current_Pos = origin_Pos + dir * time;
	current_Pos.y = player->position.y;
	return current_Pos;
}

void PlayMode::put_away_bullet(Bullet bullet)
{
	bullet.transform->position.y = 100;
	bullet.transform->scale = glm::vec3(0);
	one_bullet_timer = 0;
	bullet_current_index++;
	bullet_current_index %= 6;
	bullet_current_time = 0;
	(*(bullets.begin() + bullet_current_index)).player_pos = player->position;
}

void PlayMode::hit_player()
{
	std::cout << player_hp->scale.x << std::endl;
	if (player_hp->scale.x > 0.0001f)
	{
		player_hp->scale.x -= max_player_hp * 0.2f;
	}
}