#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

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
	meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });
Load<GLuint> scene_texture(LoadTagEarly, []() -> GLuint const *
						   {

	//make a 1-pixel white texture to bind by default:
	GLuint *tex = new GLuint;
	glGenTextures(1, tex);

	glBindTexture(GL_TEXTURE_2D, *tex);
	//std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	std::vector< glm::u8vec4 > tex_data;
	glm::uvec2 size;
	load_png("Tileset_demo.png", &size, &tex_data, OriginLocation::LowerLeftOrigin);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex; });

Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const *
						  { return new Scene(
								data_path("cube.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name)
								{
									
									Mesh const mesh = meshes->lookup(mesh_name);

									scene.drawables.emplace_back(transform);
									Scene::Drawable &drawable = scene.drawables.back();
									drawable.pipeline = lit_color_texture_program_pipeline;
									drawable.pipeline.type = mesh.type;
									drawable.pipeline.start = mesh.start;
									drawable.pipeline.count = mesh.count;
									//PlayMode::current_drawable_name = transform.name;
									//drawable.pipeline.textures[0].texture= *scene_texture;

									drawable.pipeline.vao = meshes_for_lit_color_texture_program; }); });

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const *
									   { return new Sound::Sample(data_path("dusty-floor.opus")); });

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	Platform newPlatform1;
	newPlatform1.pos = glm::vec3{-1.2f, 0, 2.0f};
	newPlatform1.height = 1.0f;
	newPlatform1.width = 1.5f;
	platforms.emplace_back(newPlatform1);
	Platform newPlatform2;
	newPlatform2.pos = glm::vec3{2.8f, 0, 3.4f};
	newPlatform2.height = 0.7f;
	newPlatform2.width = 2.2f;
	platforms.emplace_back(newPlatform2);
	Platform newPlatform3;
	newPlatform3.pos = glm::vec3{-2.9f, 0, 3.4f};
	newPlatform3.height = 1.2f;
	newPlatform3.width = 2.0f;
	platforms.emplace_back(newPlatform3);

	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Player")
		{
			player = &transform;
			start_point = player->position;
			player_origin_scale = player->scale;
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
			if (bullet_index < 3)
				current_bullets.emplace_back(bullet);

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
		else if (transform.name == "Component")
		{
			component = &transform;
		}
	}

	for (auto &bullet : bullets)
	{
		bullet.player_pos = player->position;
	}
	for (auto &bullet : current_bullets)
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
		else if (evt.key.keysym.sym == SDLK_e)
		{
			keyatk.downs += 1;
			keyatk.pressed = true;
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
		else if (evt.key.keysym.sym == SDLK_e)
		{
			keyatk.pressed = false;
			attack = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{
	// show dialogue
	// show_dialogue();

	// player die
	if (player_hp->scale.x <= 0.0001f)
	{
		player->scale = glm::vec3(0);
	}

	// boss die
	if (boss_hp->scale.x <= 0.0001f)
	{
		boss->scale = glm::vec3(0);
		// put_away_bullet(current_bullet);
	}
	else
	{
		// bullet attack
		for (auto &bullet : current_bullets)
		{

			if (!bullet.hit_player && bullet.transform->position.x < player->position.x + 0.5f && bullet.transform->position.x > player->position.x)
			{
				hit_player();
				bullet.hit_player = true;
				put_away_bullet(bullet);
			}
		}
		// boss status
		switch (boss_status)
		{
		case Melee:

			break;
		case Shoot:
			// shoot
			current_bullets_index = 0;
			bullet_total_time += bullet_speed * elapsed;
			if (shooting)
			{
				if (bullet_total_time > 0)
				{
					if (current_bullets.begin()->bullet_current_time < 20)
					{
						auto bi = current_bullets.begin();
						std::advance(bi, 0);
						bi->transform->scale = glm::vec3(0.15f);
						bi->transform->position.y = player->position.y;
						bi->bullet_current_time += bullet_speed * elapsed;
						bi->transform->position = bullet_current_Pos(boss->position, current_bullets.begin()->player_pos, current_bullets.begin()->bullet_current_time);
					}
					else
					{
						current_bullets.begin()->bullet_current_time = 0;
					}
				}
				if (bullet_total_time > 1)
				{
					auto bi = current_bullets.begin();
					std::advance(bi, 1);
					if (bi->bullet_current_time < 20)
					{
						bi->transform->scale = glm::vec3(0.15f);
						bi->transform->position.y = player->position.y;
						bi->bullet_current_time += bullet_speed * elapsed;
						bi->transform->position = bullet_current_Pos(boss->position, bi->player_pos, bi->bullet_current_time);
					}
					else
					{
						bi->bullet_current_time = 0;
					}
				}
				if (bullet_total_time > 2)
				{

					auto bi = current_bullets.begin();
					std::advance(bi, 2);
					if (bi->bullet_current_time < 20)
					{
						bi->transform->scale = glm::vec3(0.15f);
						bi->transform->position.y = player->position.y;
						bi->bullet_current_time += bullet_speed * elapsed;
						bi->transform->position = bullet_current_Pos(boss->position, bi->player_pos, bi->bullet_current_time);
					}
					else
					{
						bi->bullet_current_time = 0;
					}
				}
			}
			if (bullet_total_time > 20)
			{

				put_away_bullet(*current_bullets.begin());
				shooting = false;
			}
			if (bullet_total_time > 23)
			{
				auto bi = current_bullets.begin();
				std::advance(bi, 1);
				put_away_bullet(*bi);
			}
			if (bullet_total_time > 25)
			{
				auto bi = current_bullets.begin();
				std::advance(bi, 2);
				put_away_bullet(*bi);
				bullet_total_time = 0;
				shooting = true;
			}

			break;
		default:

			break;
		}
	}
	// player attack
	if (keyatk.pressed && !attack && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).z < boss->position.z + 0.5f && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).z > boss->position.z && ((face_right && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).x < boss->position.x + 0.8f && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).x > boss->position.x) || (!face_right && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).x < boss->position.x && (component->make_local_to_world() * glm::vec4(component->position, 1.0f)).x > boss->position.x - 0.8f)))
	{
		attack = true;
		hit_boss();
	}
	// move camera:
	{
		// combine inputs into a move:
		constexpr float PlayerSpeed = 2.0f;
		// move left and right
		float move = 0.0f;
		if (keya.pressed && !keyd.pressed)
		{
			player->scale.x = -player_origin_scale.x;
			face_right = false;
			move = -1.0f;
		}

		if (!keya.pressed && keyd.pressed)
		{
			move = 1.0f;
			player->scale.x = player_origin_scale.x;
			face_right = true;
		}

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
		float hori_move = move * PlayerSpeed * elapsed;

		jump_velocity += gravity * elapsed;
		float vert_move = jump_velocity * elapsed;

		// std::cout << first_jump << ", " << second_jump << ", " << jump_interval << "\n";
		// std::cout << jump_velocity << "\n";

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_up = frame[1];
		// glm::vec3 frame_forward = -frame[2];

		if (on_platform())
		{
			first_jump = false;
			second_jump = false;
			jump_velocity = 0;
			jump_signal = false;
		}

		if (hit_platform())
		{
			jump_velocity = 0;
		}

		glm::vec3 expected_position = player->position + hori_move * frame_right + vert_move * frame_up;
		land_on_platform(expected_position);
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
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	// {
	// 	text.Draw(text_program, base_line, 100.0f, 480.0f, drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.4f);
	// 	text.Draw(text_program, choice_lines[0], 100.0f, 250.0f, drawable_size, glm::vec3(1.0f, 1.0f, 0.3f), 0.4f);
	// 	text.Draw(text_program, choice_lines[1], 100.0f, 210.0f, drawable_size, glm::vec3(1.0f, 1.0f, 0.3f), 0.4f);
	// 	text.Draw(text_program, choice_lines[2], 100.0f, 170.0f, drawable_size, glm::vec3(1.0f, 1.0f, 0.3f), 0.4f);
	// 	text.Draw(text_program, "Next - Space; Choices - 1/2/3", 250.0f, 0.0f, drawable_size, glm::vec3(0.4f, 0.4f, 0.4f), 0.4f);
	// }

	GL_ERRORS();
}

glm::vec3 world_coords(Scene::Transform *block)
{
	auto world_block = block->make_local_to_world() * glm::vec4(block->position, 1.0f);
	return world_block;
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
	// std::cout << "dir:(" << dir.x << "," << dir.y << "," << dir.z << ")";
	//  std::cout << "position:(" << current_Pos.x << "," << current_Pos.y << "," << current_Pos.z << ")"
	//  		  << std::endl;
	return current_Pos;
}

void PlayMode::put_away_bullet(Bullet bullet)
{
	// put away bullet
	bullet.transform->position.y = 100;
	bullet.transform->scale = glm::vec3(0);
	int away_index = 0;
	for (auto blt : current_bullets)
	{
		if (bullet.index == blt.index)
		{
			auto bi = current_bullets.begin();
			std::advance(bi, away_index);
			current_bullets.erase(bi);
			break;
		}
		away_index++;
	}

	// generate new one
	bullet_current_index++;
	bullet_current_index %= 6;
	// bullet.bullet_current_time = 0;
	current_bullets.emplace_back(*(bullets.begin() + bullet_current_index));
	auto bi = current_bullets.begin();
	std::advance(bi, 2);
	bi->player_pos = player->position;
	bi->hit_player = false;
}

void PlayMode::hit_player()
{
	if (player_hp->scale.x > 0.0001f)
	{
		player_hp->scale.x -= max_player_hp * 0.2f;
	}
}

void PlayMode::hit_boss()
{
	if (boss_hp->scale.x > 0.0001f)
	{
		boss_hp->scale.x -= max_boss_hp * 0.1f;
	}
}

bool PlayMode::on_platform()
{
	for (auto platform : platforms)
	{
		if (player->position.z == platform.pos.z + platform.height / 2)
		{
			return true;
		}
	}
	return player->position.z == start_point.z;
}

bool PlayMode::hit_platform()
{
	for (auto platform : platforms)
	{
		if (player->position.z == platform.pos.z - platform.height / 2)
		{
			return true;
		}
	}
	return false;
}

void PlayMode::land_on_platform(glm::vec3 expected_position)
{
	// std::cout << "\n"
	// 		  << player->position.x << " ," << player->position.y << " ," << player->position.z;
	for (auto platform : platforms)
	{
		// std::cout << "\n" << outer_block -> name << "position z " << world_coords(outer_block).z ;
		// std::cout << "\n" << outer_block -> name << "scale x" << outer_block->scale.x;
		// std::cout << "\n" << expected_position.z << "z";
		if (std::abs(expected_position.x - platform.pos.x) < platform.width / 2 &&
			std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
		{
			if (std::abs(player->position.x - platform.pos.x) < platform.width / 2 && expected_position.x > platform.pos.x)
			{
				if (expected_position.z > platform.pos.z)
				{
					if (std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
					{
						expected_position.z = platform.pos.z + platform.height / 2;
					}
				}
				if (expected_position.z < platform.pos.z)
				{
					if (std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
					{
						expected_position.z = platform.pos.z - platform.height / 2;
					}
				}
			}
			if (std::abs(player->position.x - platform.pos.x) < platform.width / 2 && expected_position.x < platform.pos.x)
			{
				if (expected_position.z > platform.pos.z)
				{
					if (std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
					{
						expected_position.z = platform.pos.z + platform.height / 2;
					}
				}
				if (expected_position.z < platform.pos.z)
				{
					if (std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
					{
						expected_position.z = platform.pos.z - platform.height / 2;
					}
				}
			}
			if (std::abs(player->position.x - platform.pos.x) >= platform.width / 2)
			{
				if (player->position.x < platform.pos.x)
				{
					expected_position.x = platform.pos.x - platform.width / 2;
				}
				if (player->position.x > platform.pos.x)
				{
					expected_position.x = platform.pos.x + platform.width / 2;
				}
			}
		}
	}
	// camera->transform->position += move.x * frame_right + move.y * frame_forward;
	// player->position += move.x * frame_right + move.y * frame_forward + move.z * frame_up;
	camera->transform->position += expected_position - player->position;
	player->position = expected_position;
	if (expected_position.z < start_point.z)
	{
		expected_position.z = start_point.z;
	}
	camera->transform->position += expected_position - player->position;
	player->position = expected_position;
}
/*
void PlayMode::show_dialogue()
{
	if (line_index <= choices.choiceLibrary.size() && line_index >= 0)
	{
		// wait until player input, then show next text
		if (showtext)
		{
			//  type:1-single line

			base_line = choices.GetChoice(line_index).baseChoice.context;
			if (choices.GetChoice(line_index).dataPath.length() > 1)
			{
				Sound::stop_all_samples();
				voiceover = Sound::play_3D(*choices.GetChoice(line_index).voice, 1.0f, glm::vec3(0, 0, 0), 10.0f);
			}

			if (choices.GetChoice(line_index).type == 1)
			{
				for (auto &line : choice_lines)
				{
					line = "";
				}
			}
			// 2-choose
			if (choices.GetChoice(line_index).type == 2)
			{
				// show all choice
				for (int i = 0; i < 3; i++)
				{
					if (i < choices.GetChoice(line_index).choiceCount)
					{
						if (choices.GetChoice(line_index).choiceCount > i)
							choice_lines[i] = std::to_string(i + 1) + ": " + choices.GetChoice(line_index).potentialChoice[i].context;
					}
					else
					{
						choice_lines[i] = "";
					}
				}
			}

			// reset player input
			showtext = false;
			space_downcount = 0;
			choice1_downcount = 0;
			choice2_downcount = 0;
			choice3_downcount = 0;
		}

		// detect player input
		if (space.released && space_downcount == 0)
		{
			if (choices.GetChoice(line_index).type == 1)
			{
				if (choices.GetChoice(line_index).baseChoice.choiceNext > 0)
				{
					space_downcount++;
					// std::cout << "next is:" << choices.GetChoice(line_index).baseChoice.choiceNext << std::endl;
					line_index = choices.GetChoice(line_index).baseChoice.choiceNext;

					showtext = true;
				}
				else
				{
					std::cout << "no next line!!" << std::endl;
				}
			}
			space.released = false;
		}

		if (choices.GetChoice(line_index).type == 2)
		{
			int choiceNum = -1;
			if (choice1.released && choice1_downcount == 0)
			{
				choice1_downcount++;
				choiceNum = 0;
				choice1.released = false;
			}
			if (choice2.released && choice2_downcount == 0 && choices.GetChoice(line_index).choiceCount > 1)
			{
				choice2_downcount++;
				choiceNum = 1;
				choice2.released = false;
			}

			if (choice3.released && choice3_downcount == 0 && choices.GetChoice(line_index).choiceCount > 2)
			{
				choice3_downcount++;
				choiceNum = 2;
				choice3.released = false;
			}

			// show next line
			if (choiceNum >= 0)
			{
				if (choices.GetChoice(line_index).potentialChoice[choiceNum].choiceNext > 0)
				{
					line_index = choices.GetChoice(line_index).potentialChoice[choiceNum].choiceNext;
					showtext = true;
				}
				else
				{
					// std::cout << "no choose content!!" << std::endl;
				}
			}
		}
	}
}
*/