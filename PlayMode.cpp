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
									transform->min=mesh.min;
									transform->max=mesh.max;
								//	std::cout<<"transform min:"<<transform->min.x<<", max:"<<transform->max.x<<std::endl;

									scene.drawables.emplace_back(transform);
									Scene::Drawable &drawable = scene.drawables.back();
									drawable.pipeline = lit_color_texture_program_pipeline;
									if(drawable.transform->name == "Frame01" || drawable.transform->name == "Frame03" || drawable.transform->name == "Frame05")
										drawable.pipeline = lit_color_texture_trans_program_pipeline;
									drawable.pipeline.type = mesh.type;
									drawable.pipeline.start = mesh.start;
									drawable.pipeline.count = mesh.count;
									//PlayMode::current_drawable_name = transform.name;
									//drawable.pipeline.textures[0].texture= *scene_texture;

									drawable.pipeline.vao = meshes_for_lit_color_texture_program; }); });

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const *
									   { return new Sound::Sample(data_path("dusty-floor.opus")); });

Load<Sound::Sample> voice_01_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice01.wav")); });

Load<Sound::Sample> voice_02_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice02.wav")); });

Load<Sound::Sample> sound_01_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound01.wav")); });

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Player")
		{
			player = &transform;
			// player->position = glm::vec3{76.828f, 5.8671f, 1.4484f};
			//  player->scale = glm::vec3{0.15f, 0.15f, 0.15f};
			start_point = player->position;
			start_point.z -= 1.0f;
			player_origin_scale = player->scale;
		}

		else if (transform.name == "Boss")
		{
			level1_boss.transform = &transform;
			current_boss = &level1_boss;
		}

		else if (transform.name.find("SubUV") != std::string::npos)
		{
			transform.scale = glm::vec3(0.0f);
			subuv.subtransforms.emplace_back(&transform);
		}

		else if (transform.name.find("SwordAttack") != std::string::npos)
		{
			transform.scale = glm::vec3(0.0f);
			weapon_subuv.subtransforms.emplace_back(&transform);
		}

		else if (transform.name.find("Boss1Sub") != std::string::npos)
		{
			transform.scale = glm::vec3(0.0f);
			boss_subuv.subtransforms.emplace_back(&transform);
		}

		else if (transform.name.find("Bullet") != std::string::npos)
		{
			Bullet bullet;
			bullet.index = bullet_index;
			bullet.transform = &transform;
			bullet.transform->scale = glm::vec3(0);
			bullet.original_pos = bullet.transform->position;

			bullets.emplace_back(bullet);
			if (bullet_index < 3)
				current_bullets.emplace_back(bullet);

			bullet_index++;
		}
		else if (transform.name == "BossHp")
		{
			boss_hp = &transform;
			boss_hp->position.z += 5;
		}
		else if (transform.name == "PlayerHp")
		{
			player_hp = &transform;
			player_hp->position.z += 5;
		}
		else if (transform.name == "PlayerFuel")
		{
			player_fuel = &transform;
			max_fuel_scale = player_fuel->scale.x;
		}
		else if (transform.name == "Component")
		{
			component = &transform;
			component_scale = component->scale;
			component->scale = glm::vec4(0);
		}
		else if (transform.name == "BossAttack")
		{
			level1_boss_weapon.transform = &transform;
			current_boss_weapon = &level1_boss_weapon;
		}
		else if (transform.name == "PlayerAttack")
		{
			player_atk_cpnt = &transform;
		}
		else if (transform.name == "Cage")
		{
			Cage cage;
			cage.index = cage_index;
			cage.transform = &transform;
			cages.emplace_back(cage);
			cage_index++;
		}
		else if (transform.name == "Boots")
		{
			boots = &transform;
		}
		else if (transform.name == "ComponentBoots")
		{
			component_boots = &transform;
			boots_scale = component_boots->scale;
			component_boots->scale = glm::vec4(0);
		}
		else if (transform.name.find("Collider") != std::string::npos)
		{
			Platform platform;
			platform.name = transform.name;
			platform.pos = transform.position;
			// platform.pos = transform.make_local_to_world() * glm::vec4(transform.position, 1.0f);
			platform.width = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).x - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).x);
			platform.height = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).z - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).z);
			platforms.emplace_back(platform);
		}
		else if (transform.name.find("Fragile") != std::string::npos)
		{
			Platform platform;
			platform.name = transform.name;
			// platform.pos = transform.position;
			platform.pos = transform.make_local_to_world() * glm::vec4(transform.position, 1.0f);
			platform.width = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).x - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).x);
			platform.height = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).z - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).z);
			platforms.emplace_back(platform);
			// 			if (transform.name == "Fragile5")
			// {
			// 	fragile5 = &transform;
			// }
		}
		else if (transform.name == "Final_Boss")
		{
			final_boss.transform = &transform;
			// current_boss = &final_boss;
		}
		else if (transform.name == "FinalBossAttack")
		{
			final_boss_weapon.transform = &transform;
			// current_boss_weapon = &final_boss_weapon;
		}
		else if (transform.name.find("Teleport") != std::string::npos)
		{
			teleportPos.emplace_back(&transform);
		}
		// add a real component jetpack
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
	camera->transform->position = glm::vec3{player->position.x + 2.5722f, -30.0f, player->position.z + 1.3373f};

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
			if (player_status != PlayerStatus::JumpLoop && player_status != PlayerStatus::JumpStart)
				player_status = PlayerStatus::MoveLeft;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			keyd.downs += 1;
			keyd.pressed = true;
			if (player_status != PlayerStatus::JumpLoop && player_status != PlayerStatus::JumpStart)
				player_status = PlayerStatus::MoveRight;
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
			weapon_status = WeaponStatus::NormalAttack;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.downs += 1;
			space.pressed = true;
			if (first_jump == false)
				player_status = PlayerStatus::JumpStart;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			keya.pressed = false;
			if (player_status != PlayerStatus::JumpLoop && player_status != PlayerStatus::JumpStart)
				player_status = PlayerStatus::Idle;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			keyd.pressed = false;
			if (player_status != PlayerStatus::JumpLoop && player_status != PlayerStatus::JumpStart)
				player_status = PlayerStatus::Idle;
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
	update_player_status();
	if (get_weapon)
		update_weapon_status();

	// get weapon
	if (!get_weapon && hit_detect_SAT(player, player_atk_cpnt).overlapped)
	{
		get_weapon = true;
		component->scale = component_scale;
		player_atk_cpnt->scale = glm::vec4(0);
	}

	// hit cage
	if (get_weapon && !cages.begin()->isDestroied && keyatk.pressed && !attack && hit_detect_SAT(component, cages.begin()->transform).overlapped)
	{
		attack = true;
		cages.begin()->isDestroied = true;
		cages.begin()->transform->scale = glm::vec4(0);
		sound = Sound::play_3D(*sound_01_sample, 1.0f, cages.begin()->transform->position);
	}

	//  get boots
	if (get_weapon && !cages.begin()->isDestroied && keyatk.pressed && !attack && hit_detect_SAT(player, cages.begin()->transform).overlapped)
	{
		// std::cout << "test collider : overlapped ......................................" << std::endl;
	}

	if (!hasBoots && cages.begin()->isDestroied && hit_detect(player, boots).overlapped)
	{
		hasBoots = true;
		boots->scale = glm::vec4(0);
		// component_boots->scale = boots_scale;
		sound = Sound::play_3D(*voice_01_sample, 1.0f, cages.begin()->transform->position);
	}

	if (second_jump && hasBoots)
	{
		boots_timer = std::min(1.0f, boots_timer + elapsed * 5);
		component_boots->scale = boots_scale * boots_timer;
	}

	// player die
	if (player_hp->scale.x <= 0.0001f)
	{
		player->scale = glm::vec3(0);
		player_die = true;
	}

	// boss die
	if (boss_hp->scale.x <= 0.0001f)
	{
		// current_boss->transform->scale = glm::vec3(0);
		for (auto &bullet : current_bullets)
		{
			put_away_bullet(bullet);
		}
		// get jet pack and replace boots
		if (!hasJetPack)
		{
			hasJetPack = true;
			hasBoots = false;
		}

		if (!current_boss->die)
			current_boss->die = level1_boss_dead();

		current_boss_weapon->transform->scale = glm::vec3(0);
	}
	else
	{
		// bullet attack
		for (auto &bullet : current_bullets)
		{

			if (!bullet.hit_player && hit_detect(player, bullet.transform).overlapped)
			{
				hit_player();
				bullet.hit_player = true;
				put_away_bullet(bullet);
			}
		}
		// boss weapon attack
		if (hit_detect(player, current_boss_weapon->transform).overlapped)
		{
			if (current_boss_weapon->timer == 0)
			{
				hit_player();
				current_boss_weapon->timer++;
			}
			if (current_boss_weapon->timer > 50)
			{
				current_boss_weapon->timer = 1;
				hit_player();
			}
			else
			{
				current_boss_weapon->timer++;
			}
		}
		else
		{
			current_boss_weapon->timer = 0;
		}
		// update boss status
		update_boss_status();

		// boss status
		switch (boss_status)
		{
		case Idle:
		{
			// deal with bullet
			if (!finish_bullet)
			{
				shooting1 = true;
				shooting2 = true;
				shooting3 = true;
				hit1 = false;
				hit2 = false;
				hit3 = false;

				finish_bullet = true;
			}
			break;
		}
		case Melee:
		{
			// deal with bullet
			if (!finish_bullet)
			{
				shooting1 = true;
				shooting2 = true;
				shooting3 = true;
				hit1 = false;
				hit2 = false;
				hit3 = false;

				finish_bullet = true;
			}
			// boss move towards to the player
			glm::vec3 dir = glm::normalize(player->position - current_boss->transform->position);
			current_boss->transform->position.x += dir.x * current_boss->speed * elapsed;
			break;
		}
		case Shoot:
		{
			// timer
			if (finish_bullet)
			{
				for (auto bullet : current_bullets)
				{
					bullet.player_pos = player->position;
				}
			}
			current_bullets_index = 0;
			bullet_total_time += bullet_speed * elapsed;
			// start new round
			if (bullet_total_time > 15)
			{
				bullet_total_time = 0;
				shooting1 = true;
				shooting2 = true;
				shooting3 = true;
				hit1 = false;
				hit2 = false;
				hit3 = false;

				// count for teleport
				if (current_boss->transform->name == final_boss.transform->name)
				{
					if (count_for_teleport > 1)
					{
						// start teleport
						count_for_teleport = 0;
						ready_to_teleport = true;
					}
					else
					{
						if (!ready_to_teleport)
						{
							count_for_teleport++;
						}
					}
				}
			}

			break;
		}
		default:
		{
			break;
		}
		}

		// teleport, the judgment of whether teleport is in this function
		teleport();
		// shoot, for deal with the last bullets before switch the attack mode, the bullets can't write in the shooting status
		if (shooting1 && !hit1 && bullet_total_time > 0)
		{
			if (current_bullets.begin()->bullet_current_time < 20)
			{
				auto bi = current_bullets.begin();
				std::advance(bi, 0);
				bi->transform->scale = glm::vec3(0.1f);
				bi->transform->position.y = player->position.y;
				bi->bullet_current_time += bullet_speed * elapsed;
				bi->transform->position = bullet_current_Pos(current_boss->transform->position, current_bullets.begin()->player_pos, current_bullets.begin()->bullet_current_time);
			}
			else
			{
				current_bullets.begin()->transform->scale = glm::vec3(0);
				current_bullets.begin()->transform->position = current_bullets.begin()->original_pos;
				current_bullets.begin()->bullet_current_time = 0;
			}
		}
		if (shooting2 && !hit2 && bullet_total_time > 1)
		{
			auto bi = current_bullets.begin();
			std::advance(bi, 1);
			if (bi->bullet_current_time < 20)
			{
				bi->transform->scale = glm::vec3(0.1f);
				bi->transform->position.y = player->position.y;
				bi->bullet_current_time += bullet_speed * elapsed;
				bi->transform->position = bullet_current_Pos(current_boss->transform->position, bi->player_pos, bi->bullet_current_time);
			}
			else
			{
				bi->transform->scale = glm::vec3(0);
				bi->transform->position = bi->original_pos;
				bi->bullet_current_time = 0;
			}
		}
		if (shooting3 && !hit3 && bullet_total_time > 2)
		{

			auto bi = current_bullets.begin();
			std::advance(bi, 2);
			if (bi->bullet_current_time < 20)
			{
				bi->transform->scale = glm::vec3(0.1f);
				bi->transform->position.y = player->position.y;
				bi->bullet_current_time += bullet_speed * elapsed;
				bi->transform->position = bullet_current_Pos(current_boss->transform->position, bi->player_pos, bi->bullet_current_time);
			}
			else
			{
				bi->transform->scale = glm::vec3(0);
				bi->transform->position = bi->original_pos;
				bi->bullet_current_time = 0;
			}
		}
		// clear 1st bullet
		if (!hit1 && bullet_total_time > 10)
		{
			put_away_bullet(*current_bullets.begin());
			shooting1 = false;
		}
		if (!hit2 && bullet_total_time > 12)
		{
			auto bi = current_bullets.begin();
			std::advance(bi, 1);
			put_away_bullet(*bi);
			shooting2 = false;
		}
		if (!hit3 && bullet_total_time > 14)
		{
			auto bi = current_bullets.begin();
			std::advance(bi, 2);
			put_away_bullet(*bi);
			shooting3 = false;
		}
	}
	// player attack
	if (get_weapon &&
		keyatk.pressed &&
		!attack && hit_detect_SAT(component, current_boss->transform).overlapped)
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

		float gravity = -4.0f;
		if (jetpack_on && jetpack_fuel > 0)
		{
			gravity = -2.0f;
		}

		if (space.pressed)
		{
			if (!first_jump)
			{
				jump_signal = false;
				first_jump = true;
				jump_velocity = 3.0f;
			}
			else if (first_jump && !second_jump && jump_signal && hasBoots)
			{
				jump_signal = false;
				second_jump = true;
				jump_velocity = 3.0f;
				// add dog and sound
				sound = Sound::play_3D(*voice_02_sample, 1.0f, player->position);
			}
			else if (hasJetPack && !jetpack_on)
			{
				jetpack_on = true;
				// jump_velocity = jetpack_max_speed;
				hover_max_time = hover_full_fuel_time / jetpack_max_fuel * jetpack_fuel;
			}
			else
			{
				// do nothing, later replace with other possibilities
			}
		}

		if (!space.pressed)
		{
			jump_signal = true;
			jetpack_on = false;
		}

		if (hasJetPack && jetpack_fuel > 0 && jetpack_on)
		{
			jetpack_fuel -= elapsed;
		}

		if (jetpack_fuel <= 0)
		{
			jetpack_on = false;
		}
		// print the jetpack fuel
		// std::cout << jetpack_fuel << "\n";

		// if (down.pressed && !up.pressed)
		// 	move.y = -1.0f;
		// if (!down.pressed && up.pressed)
		// 	move.y = 1.0f;

		// make it so that moving diagonally doesn't go faster:
		float hori_move = move * PlayerSpeed * elapsed;

		jump_velocity += gravity * elapsed;
		float vert_move = jump_velocity * elapsed;
		if (jetpack_on)
		{
			vert_move += jetpack_max_speed * elapsed;
		}
		// if (jetpack_on && hover_time < hover_max_time && jump_velocity < 0)
		// {
		// 	jump_velocity = 0;
		// 	vert_move = 0;
		// 	hover_time += elapsed;
		// }

		// std::cout << "hover time" << hover_time << "\n";

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
			hover_time = 0;
			jump_signal = false;

			jetpack_on = false;
			if (jetpack_fuel < jetpack_max_fuel)
			{
				jetpack_fuel += 2 * elapsed;
			}
		}

		if (!hasJetPack) {
			player_fuel->scale.x = 0.001f;
		}
		else {
			if (jetpack_fuel <= 0) {
				player_fuel->scale.x = 0.001f;
			}
			else {
				player_fuel->scale.x = max_fuel_scale * jetpack_fuel / jetpack_max_fuel;
			}
		}

		if (hit_platform())
		{
			jump_velocity = 0;
		}

		if (stage_changing) {
			hori_move = 0;
			if (vert_move > 0) vert_move = 0;
			if (jump_velocity > 0) jump_velocity = 0;
			stage_change_timer += elapsed;
			if (stage_change_timer > 2.0f) {
				stage_changing = false;
			}
		}
		
		glm::vec3 expected_position = player->position + hori_move * frame_right + vert_move * frame_up;

		// check change stage
		expected_position = check_change_stage(expected_position);

		/*
		Mesh const &playermesh = meshes->lookup(player->name);
		for(GLuint i=0; i< playermesh.count; i++){
			std::cout<<glm::to_string(playermesh.verticesList[i]) <<std::endl;
		}
		std::cout<<"end<<<<<<<<<" << std::endl; */
		land_on_platform(expected_position);
		std::cout << player->position.x << "," << player->position.y << "," << player->position.z << "\n";
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

	// set up light type and position for lit_color_texture_program:w
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

	GL_ERRORS();
}

glm::vec3 world_coords(Scene::Transform *block)
{
	auto world_block = block->make_local_to_world() * glm::vec4(block->position, 1.0f);
	return world_block;
}

glm::vec3 PlayMode::check_change_stage(glm::vec3 expected_position) {
	if (player_stage == PlayerStage::InitialStage && level1_boss.die)
		{
			if (player->position.x > 19.0f && player->position.z > 7.0f)
			{
				player_stage = PlayerStage::JumpGame;
				camera->transform->position.x += 38.0f - player->position.x;
				camera->transform->position.z += 5.3f - player->position.z;
				player->position.x = 38.0f;
				player->position.z = 5.3f;
				expected_position.x = 38.0f;
				expected_position.z = 5.3f;
				stage_changing = true;
				stage_change_timer = 0.0f;
			}
		}

	if (player_stage == PlayerStage::JumpGame)
		{
			if (player->position.x > 49.0f && player->position.z > 55.0f)
			{
				player_stage = PlayerStage::BossTeleport;
				camera->transform->position.x = 63.0f;
				camera->transform->position.z = 5.0f;
				player->position.x = 63.0f;
				player->position.z = 5.0f;
				expected_position.x = 63.0f;
				expected_position.z = 5.0f;
				boss_hp->scale.x = 1;
				current_boss = &final_boss;
				current_boss_weapon = &final_boss_weapon;
				stage_changing = true;
				stage_change_timer = 0.0f;
			}
		}
		return expected_position;
}
glm::vec3 PlayMode::bullet_current_Pos(glm::vec3 origin_Pos, glm::vec3 final_Pos, float time)
{
	glm::vec3 dir = glm::normalize(final_Pos - origin_Pos);
	glm::vec3 current_Pos = origin_Pos + dir * time;
	current_Pos.y = player->position.y;
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
			// current_bullets.erase(bi);

			bullet_current_index += 3;
			bullet_current_index %= 9;
			Bullet new_bullet = *(bullets.begin() + bullet_current_index);

			bi->index = new_bullet.index;
			bi->transform = new_bullet.transform;
			bi->transform->position = current_boss->transform->position;
			bi->final_pos = player->position;
			bi->bullet_current_time = 0;
			bi->player_pos = player->position;
			bi->hit_player = false;
			break;
		}
		away_index++;
	}
	switch (away_index)
	{
	case 0:
		hit1 = true;
		break;
	case 1:
		hit2 = true;
		break;
	case 2:
		hit3 = true;
		break;
	default:
		break;
	}
}

void PlayMode::update_player_status()
{
	subuv.anim_timer += 0.1f * subuv.speed;

	uint32_t bit = 1;
	while (true)
	{
		if (subuv.bitmask >> bit == 0)
			break;
		bit++;
	}

	switch (player_status)
	{
	case PlayerStatus::Idle:
		subuv.start_index = 0;
		subuv.range = 7;
		subuv.speed = 1.25f;
		break;
	case PlayerStatus::MoveLeft:
		subuv.start_index = 8;
		subuv.range = 7;
		subuv.speed = 2.5f;
		break;
	case PlayerStatus::MoveRight:
		subuv.start_index = 16;
		subuv.range = 7;
		subuv.speed = 2.5f;
		break;
	case PlayerStatus::JumpStart:
		subuv.start_index = 24;
		subuv.range = 4;
		subuv.speed = 1.75f;
		break;
	case PlayerStatus::JumpLoop:
		subuv.start_index = 29;
		subuv.range = 2;
		break;
	case PlayerStatus::JumpEnd:
		subuv.start_index = 32;
		subuv.range = 3;
		subuv.speed = 1.75f;
		break;
	default:
		subuv.start_index = 0;
		subuv.range = 7;
	}

	if (subuv.anim_timer > 1)
	{
		subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit++;
		if (bit - 1 >= subuv.start_index + subuv.range)
		{
			if (player_status == PlayerStatus::JumpStart)
			{
				player_status = PlayerStatus::JumpLoop;
				bit = 30; // 29+1
			}
			else if (player_status == PlayerStatus::JumpEnd)
			{
				player_status = PlayerStatus::Idle;
				bit = 1;
			}
			else
				bit = subuv.start_index + 1;
		}
		subuv.bitmask = 1ULL << (bit - 1);
		subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
		subuv.anim_timer = 0.0f;
	}
	else if (bit - 1 >= subuv.start_index + subuv.range || bit - 1 < subuv.start_index)
	{
		subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit = subuv.start_index + 1;
		subuv.bitmask = 1ULL << (bit - 1);
		subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
		subuv.anim_timer = 0.0f;
	}
}

void PlayMode::update_weapon_status()
{
	weapon_subuv.anim_timer += 0.1f * weapon_subuv.speed;

	uint32_t bit = 1;
	while (true)
	{
		if (weapon_subuv.bitmask >> bit == 0)
			break;
		bit++;
	}

	switch (weapon_status)
	{
	case WeaponStatus::Idle:
		weapon_subuv.start_index = 7;
		weapon_subuv.range = 0;
		weapon_subuv.speed = 1.25f;
		break;
	case WeaponStatus::NormalAttack:
		weapon_subuv.start_index = 0;
		weapon_subuv.range = 6;
		weapon_subuv.speed = 2.2f;
		break;
	default:
		weapon_subuv.start_index = 7;
		weapon_subuv.range = 0;
	}

	if (weapon_status != WeaponStatus::Idle && weapon_subuv.anim_timer > 1)
	{
		weapon_subuv.subtransforms[6]->scale = glm::vec3(0.0f);
		weapon_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit++;
		if (bit - 1 > weapon_subuv.start_index + weapon_subuv.range)
		{
			// bit = subuv.start_index + 1;
			bit = 1;
			weapon_status = WeaponStatus::Idle;
			weapon_subuv.subtransforms[6]->scale = glm::vec3(1.5f);
		}
		else
		{
			weapon_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.5f);
		}
		weapon_subuv.bitmask = 1ULL << (bit - 1);
		weapon_subuv.anim_timer = 0.0f;
	}
	else if (weapon_status == WeaponStatus::Idle)
	{
		weapon_subuv.subtransforms[6]->scale = glm::vec3(1.5f);
	}
	// else if (bit - 1 >= subuv.start_index + subuv.range || bit - 1 < subuv.start_index)
	// {
	// 	subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
	// 	bit = subuv.start_index + 1;
	// 	subuv.bitmask = 1 << (bit - 1);
	// 	subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
	// 	subuv.anim_timer = 0.0f;
	// }
}

void PlayMode::hit_player()
{
	if (player_hp->scale.x > 0.0001f)
	{
		player_hp->scale.x -= max_player_hp * 0.05f;
	}
}

void PlayMode::hit_boss()
{
	if (boss_hp->scale.x > 0.0001f)
	{
		boss_hp->scale.x -= current_boss->max_hp * 0.1f;
		boss_status = BattleStatus::Attacked;
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
	for (auto platform : platforms)
	{
		// std::cout << "name:" << platform.name << ",expected pos:" << expected_position.z << ",plat pos:" << platform.pos.z << ",3:" << std::abs(expected_position.z - platform.pos.z) << "4:" << platform.height / 2 << std::endl;
		if (std::abs(expected_position.x - platform.pos.x) < platform.width / 2 &&
			std::abs(expected_position.z - platform.pos.z) < platform.height / 2)
		{
			// This is just for JumpEnd Animation
			if (player->position.z > platform.pos.z + platform.height / 2)
			{
				if (expected_position.z < platform.pos.z + platform.height / 2)
				{
					// from higher position
					player_status = PlayerStatus::JumpEnd;
					// shrink the dog
					second_jump = false;
					component_boots->scale = glm::vec3(0);
					boots_timer = 0.0f;
				}
			}
			// This is real collision detection
			if (player->position.z >= platform.pos.z + platform.height / 2)
			{
				if (expected_position.z < platform.pos.z + platform.height / 2)
				{
					// from higher position
					expected_position.z = platform.pos.z + platform.height / 2;
				}
			}
			if (player->position.z <= platform.pos.z - platform.height / 2)
			{
				if (expected_position.z > platform.pos.z - platform.height / 2)
				{
					// for lower position
					expected_position.z = platform.pos.z - platform.height / 2;
				}
				if (player->position.x < platform.pos.x - platform.width / 2 &&
					expected_position.x > platform.pos.x - platform.width / 2)
				{
					expected_position.x = platform.pos.x - platform.width / 2;
				}
				if (player->position.x > platform.pos.x + platform.width / 2 &&
					expected_position.x < platform.pos.x + platform.width / 2)
				{
					expected_position.x = platform.pos.x + platform.width / 2;
				}
			}
			else
			{
				if (player->position.x <= platform.pos.x - platform.width / 2)
				{
					if (expected_position.x > platform.pos.x - platform.width / 2)
					{
						expected_position.x = platform.pos.x - platform.width / 2;
					}
				}
				if (player->position.x >= platform.pos.x + platform.width / 2)
				{
					if (expected_position.x < platform.pos.x + platform.width / 2)
					{
						expected_position.x = platform.pos.x + platform.width / 2;
					}
				}
			}
		}
	}
	// camera->transform->position += move.x * frame_right + move.y * frame_forward;
	// player->position += move.x * frame_right + move.y * frame_forward + move.z * frame_up;

	// if (expected_position.z <= start_point.z && std::abs(expected_position.x - (-11.5f)) < 4.4f)
	// // hardcode to prevent a jump at start
	// {
	//   expected_position.z = start_point.z;
	// }
	if (expected_position.z < start_point.z)
	// hardcode to prevent a jump at start
	{
		expected_position.z = start_point.z;
	}
	if (!player_die)
	{
		camera->transform->position += expected_position - player->position;
		player->position = expected_position;
	}
}
PlayMode::HitObject PlayMode::hit_detect_SAT(Scene::Transform *obj, Scene::Transform *hit_obj)
{
	bool is_collide = true;
	// std::cout << "look for obj " << obj->name << std::endl;
	Mesh const &objmesh = meshes->lookup(obj->name);
	// std::cout << "look for hit obj " << hit_obj->name << std::endl;
	/*
		for (auto &p : objmesh.points)
		{
			//std::cout << glm::to_string(obj->make_local_to_world() * glm::vec4(p.position, 1.0f)) << std::endl;
		}*/
	Mesh const &hit_objmesh = meshes->lookup(hit_obj->name);
	for (GLuint i = 0; i < objmesh.halfEdges.size(); i++)
	{
		if (objmesh.halfEdges[i].twin == -1)
		{
			glm::vec3 p0 = obj->make_local_to_world() * glm::vec4(objmesh.points[objmesh.halfEdges[i].p0].position, 1.0f);
			glm::vec3 p1 = obj->make_local_to_world() * glm::vec4(objmesh.points[objmesh.halfEdges[i].p1].position, 1.0f);
			// std::cout << "p0 is  " << glm::to_string(p0) << std::endl;
			// std::cout << "p1 is  " << glm::to_string(p1) << std::endl;

			glm::vec3 edge = glm::normalize(p1 - p0);
			glm::vec3 axis = glm::normalize(glm::cross(edge, glm::vec3(0.0f, 1.0f, 0.0f)));
			// std::cout << "we have edges " << glm::to_string(edge) << std::endl;
			// std::cout << "we have axis " << glm::to_string(axis) << std::endl;

			std::pair<float, float> result0, result1;
			result0 = ProjectAlongVector(obj, axis);
			result1 = ProjectAlongVector(hit_obj, axis);
			// std::cout << "result0 min : " << result0.first << " , max :" << result0.second << std::endl;
			// std::cout << "result1 min : " << result1.first << " , max :" << result1.second << std::endl;

			if ((result0.first > result1.second) || (result1.first > result0.second))
			{
				is_collide = false;
				break;
			}
		}
		// std::cout<<glm::to_string(playermesh.verticesList[i]) <<std::endl;
	}
	////std::cout << "then    " << std::endl;
	if (is_collide)
	{
		for (GLuint i = 0; i < hit_objmesh.halfEdges.size(); i++)
		{
			if (hit_objmesh.halfEdges[i].twin == -1)
			{
				glm::vec3 p0 = obj->make_local_to_world() * glm::vec4(objmesh.points[objmesh.halfEdges[i].p0].position, 1.0f);
				glm::vec3 p1 = obj->make_local_to_world() * glm::vec4(objmesh.points[objmesh.halfEdges[i].p1].position, 1.0f);
				glm::vec3 edge = glm::normalize(p1 - p0);
				glm::vec3 axis = glm::normalize(glm::cross(edge, glm::vec3(0.0f, 1.0f, 0.0f)));
				// std::cout<<"we have eges "<< glm::to_string(axis)<<std::endl;
				std::pair<float, float> result0, result1;
				result0 = ProjectAlongVector(obj, axis);
				result1 = ProjectAlongVector(hit_obj, axis);
				// std::cout << "result0 min : " << result0.first << " , max :" << result0.second << std::endl;
				// std::cout << "result1 min : " << result1.first << " , max :" << result1.second << std::endl;
				if ((result0.first > result1.second) || (result1.first > result0.second))
				{
					is_collide = false;
					break;
				}
			}
			// std::cout<<glm::to_string(playermesh.verticesList[i]) <<std::endl;
		}
	}

	// std::cout<<"end<<<<<<<<<" << std::endl;

	PlayMode::HitObject hit_object_result;
	hit_object_result.name = hit_obj->name;
	hit_object_result.overlapped = is_collide;
	return hit_object_result;
}
PlayMode::HitObject PlayMode::hit_detect(Scene::Transform *obj, Scene::Transform *hit_obj)
{
	float obj_max_x = (obj->make_local_to_world() * glm::vec4(obj->max, 1.0f)).x;
	float obj_min_x = (obj->make_local_to_world() * glm::vec4(obj->min, 1.0f)).x;
	float obj_max_z = (obj->make_local_to_world() * glm::vec4(obj->max, 1.0f)).z;
	float obj_min_z = (obj->make_local_to_world() * glm::vec4(obj->min, 1.0f)).z;
	float hit_obj_max_x = (hit_obj->make_local_to_world() * glm::vec4(hit_obj->max, 1.0f)).x;
	float hit_obj_min_x = (hit_obj->make_local_to_world() * glm::vec4(hit_obj->min, 1.0f)).x;
	float hit_obj_max_z = (hit_obj->make_local_to_world() * glm::vec4(hit_obj->max, 1.0f)).z;
	float hit_obj_min_z = (hit_obj->make_local_to_world() * glm::vec4(hit_obj->min, 1.0f)).z;

	float minMax = 0;
	float maxMin = 0;
	if (face_right)
	{
		if (obj_max_x < hit_obj_max_x)
		{
			minMax = obj_max_x;
			maxMin = hit_obj_min_x;
		}
		else
		{
			minMax = hit_obj_max_x;
			maxMin = obj_min_x;
		}
	}
	else
	{
		if (obj_max_x < hit_obj_max_x)
		{
			minMax = obj_min_x;
			maxMin = hit_obj_min_x;
		}
		else
		{
			minMax = hit_obj_max_x;
			maxMin = obj_max_x;
		}
	}

	bool TouchX = false;
	if (face_right)
	{
		if (maxMin - minMax < 0.26f)
		{
			TouchX = true;
		}
		else
		{
			TouchX = false;
		}
	}
	else
	{
		if (maxMin - minMax < 0.5f)
		{
			TouchX = true;
		}
		else
		{
			TouchX = false;
		}
	}

	if (obj_max_z < hit_obj_max_z)
	{
		minMax = obj_max_z;
		maxMin = hit_obj_min_z;
	}
	else
	{
		minMax = hit_obj_max_z;
		maxMin = obj_min_z;
	}

	bool TouchZ = false;
	if (maxMin - minMax < 0.26f)
	{
		TouchZ = true;
	}
	else
	{
		TouchZ = false;
	}
	PlayMode::HitObject hit_object_result;
	if (TouchX && TouchZ)
	{
		hit_object_result.name = hit_obj->name;
		hit_object_result.overlapped = true;
	}
	return hit_object_result;
}

std::pair<float, float> PlayMode::ProjectAlongVector(Scene::Transform *obj, const glm::vec3 &projectionVector)
{
	std::vector<float> projectedPositions;
	Mesh const &objmesh = meshes->lookup(obj->name);
	float min = std::numeric_limits<float>::infinity();
	float max = -std::numeric_limits<float>::infinity();
	// std::cout << "doing projection" << std::endl;
	for (const auto &point : objmesh.points)
	{
		// Calculate the projection
		glm::vec3 position = obj->make_local_to_world() * glm::vec4(point.position, 1.0f);
		// std::cout << glm::to_string(position) <<std::endl;
		float dotProduct = glm::dot(position, projectionVector);
		// std::cout << dotProduct << std::endl;
		float projectionLength = glm::length(projectionVector); // squared length to avoid square root
		float projection = dotProduct / projectionLength;

		// projectedPositions.push_back(projection);
		// projectedPositions.push_back(projectionLength);
		if (projection < min)
		{
			min = projection;
		}
		if (projection > max)
		{
			max = projection;
		}
	}
	std::pair<float, float> result;

	result.first = min;
	result.second = max;
	// std::cout << result.first <<"  and  " << result.second << std::endl;
	return result;
}
glm::vec3 PlayMode::nearest_teleport()
{
	float mindis = INFINITY;

	glm::vec3 minpos;

	for (auto &telepos : teleportPos)
	{

		float dis = glm::distance(player->position, telepos->position);
		minpos = dis < mindis ? telepos->position : minpos;
		mindis = dis < mindis ? dis : mindis;
	}
	return minpos;
}
void PlayMode::update_boss_status()
{
	{ // Update boss animation
		if (current_boss->transform->name == level1_boss.transform->name)
		{
			if (player->position.x > current_boss->transform->position.x)
				current_boss->transform->scale.x = -0.424f;
			else
				current_boss->transform->scale.x = 0.424f;
		}

		boss_subuv.anim_timer += 0.1f * boss_subuv.speed;

		uint32_t bit = 1;
		while (true)
		{
			if (boss_subuv.bitmask >> bit == 0)
				break;
			bit++;
		}

		switch (boss_status)
		{
		case BattleStatus::Attacked:
			boss_subuv.start_index = 10;
			boss_subuv.range = 0;
			boss_subuv.speed = 1.0f;
			break;
		// case BattleStatus::Dead:
		// 	boss_subuv.start_index = 6;
		// 	boss_subuv.range = 3;
		// 	boss_subuv.speed = 1.5f;
		// 	break;
		default:
			boss_subuv.start_index = 0;
			boss_subuv.range = 5;
			boss_subuv.speed = 1.25f;
		}

		if (boss_status == BattleStatus::Dead)
			return;
		else if (boss_status == BattleStatus::Attacked)
		{
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit = 11;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			attacked_timer += 0.1f;
			if (attacked_timer > 1.25f)
			{
				boss_status = BattleStatus::Idle;
				attacked_timer = 0.0f;
			}
		}
		else if (boss_subuv.anim_timer > 1)
		{
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit++;
			if (bit - 1 >= boss_subuv.start_index + boss_subuv.range)
				bit = boss_subuv.start_index + 1;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			boss_subuv.anim_timer = 0.0f;
		}
		else if (bit - 1 >= boss_subuv.start_index + boss_subuv.range || bit - 1 < boss_subuv.start_index)
		{
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit = boss_subuv.start_index + 1;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			boss_subuv.anim_timer = 0.0f;
		}
	}

	if (detect_boss_status)
	{
		if (current_boss->transform->name == final_boss.transform->name)
		{
			if (glm::distance(player->position, current_boss->transform->position) < 2.5f && boss_status != BattleStatus::Attacked)
			{
				boss_status = Melee;
			}
			else if (glm::distance(player->position, current_boss->transform->position) < 8.0f && boss_status != BattleStatus::Attacked)
			{
				boss_status = Shoot;
			}
			else if (boss_status != BattleStatus::Attacked || boss_hp->scale.x <= 0.2f)
			{
				boss_status = Idle;
			}
		}
		else
		{
			if (glm::distance(player->position, current_boss->transform->position) < 2.5f && boss_status != BattleStatus::Attacked)
			{

				boss_status = Melee;
			}
			else if (glm::distance(player->position, current_boss->transform->position) < 6.0f && boss_status != BattleStatus::Attacked)
			{
				boss_status = Shoot;
			}
			else if (boss_status != BattleStatus::Attacked || boss_hp->scale.x <= 0.2f)
			{
				boss_status = Idle;
			}
		}
	}
}

bool PlayMode::level1_boss_dead()
{
	boss_subuv.anim_timer += 0.1f * boss_subuv.speed;

	uint32_t bit = 1;
	while (true)
	{
		if (boss_subuv.bitmask >> bit == 0)
			break;
		bit++;
	}

	if (bit == 10) // last frame of dead
		return true;
	else
	{
		boss_subuv.start_index = 6;
		boss_subuv.range = 3;
		boss_subuv.speed = 0.75f;

		if (boss_subuv.anim_timer > 1)
		{
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit++;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			boss_subuv.anim_timer = 0.0f;
		}

		return false;
	}
}

void PlayMode::teleport()
{
	if (current_boss->transform->name == final_boss.transform->name)
	{
		if (ready_to_teleport)
		{
			if (!arrive_new_pos)
			{
				if (teleport_timer > 0)
				{
					detect_boss_status = false;
					boss_status = Idle;
					teleport_timer -= 0.03f;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
				}
				else
				{
					teleport_timer = 0;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
					glm::vec3 nearest_tel = nearest_teleport();
					current_boss->transform->position = glm::vec3(nearest_tel.x, player->position.y, nearest_tel.z);
					arrive_new_pos = true;
				}
			}
			else
			{
				if (teleport_timer < 0.3f)
				{
					teleport_timer += 0.03f;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
				}
				else
				{
					teleport_timer = 0.3f;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
					arrive_new_pos = false;
					detect_boss_status = true;
					ready_to_teleport = false;
				}
			}

			// }
		}
	}
}
