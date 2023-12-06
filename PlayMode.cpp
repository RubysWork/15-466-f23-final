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

Load<Sound::Sample> bgm_sample(LoadTagDefault, []() -> Sound::Sample const *
							   { return new Sound::Sample(data_path("BGM.wav")); });

Load<Sound::Sample> voice_01_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice01.wav")); });

Load<Sound::Sample> voice_02_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice02.wav")); });

Load<Sound::Sample> voice_03_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice03.wav")); });

Load<Sound::Sample> voice_04_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice04.wav")); });

Load<Sound::Sample> voice_05_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice05.wav")); });

Load<Sound::Sample> voice_06_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice06.wav")); });

Load<Sound::Sample> voice_07_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Voice07.wav")); });

Load<Sound::Sample> sound_01_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound01.wav")); });

Load<Sound::Sample> sound_02_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound02.wav")); });

Load<Sound::Sample> sound_03_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound03.wav")); });

Load<Sound::Sample> sound_04_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound04.wav")); });

Load<Sound::Sample> sound_05_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound05.wav")); });

Load<Sound::Sample> sound_06_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound06.wav")); });

Load<Sound::Sample> sound_07_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound07.wav")); });

Load<Sound::Sample> sound_08_sample(LoadTagDefault, []() -> Sound::Sample const *
									{ return new Sound::Sample(data_path("Sound08.wav")); });

Load<Sound::Sample> win_sample(LoadTagDefault, []() -> Sound::Sample const *
							   { return new Sound::Sample(data_path("win.wav")); });

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	// Initialize draw text
	text.HB_FT_Init(data_path("PressStart2P-Regular.ttf").c_str(), 50);
	text_program = text.CreateTextShader();

	for (int i = 0; i < 9; ++i)
		enemy_subuv.emplace_back();

	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Player")
		{
			player = &transform;
			player->position = glm::vec3{47.3101f, 5.86705f, 56.8865f};
			//     player->scale = glm::vec3{0.15f, 0.15f, 0.15f};
			start_point = player->position;
			start_point.z -= 5.0f;
			player_origin_scale = player->scale;
		}

		else if (transform.name == "Boss")
		{
			level1_boss.transform = &transform;
			current_boss = &level1_boss;
			current_boss->hasweapon = true;
			current_boss->status = Idle;
		}

		else if (transform.name == "StartMenu")
		{
			start_menu = &transform;
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

		else if (transform.name.find("WingsSub") != std::string::npos)
		{
			transform.scale = glm::vec3(0.0f);
			wings_subuv.subtransforms.emplace_back(&transform);
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
			ori_bosshp_scale = boss_hp->scale;
			// boss_hp->position.z += 5;
		}
		else if (transform.name == "Boss1HpBG")
		{
			boss_hp_bg = &transform;
		}
		else if (transform.name == "PlayerHp")
		{
			player_hp = &transform;
			// player_hp->position.z += 5;
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
			// component->scale = glm::vec4(0);
			get_weapon = true;
		}
		else if (transform.name == "BossAttack")
		{
			BossWeapon *weapon = new BossWeapon();
			weapon->transform = &transform;
			weapon->ori_weap_scale = transform.scale;
			level1_boss.weapon = weapon;
		}
		else if (transform.name == "PlayerAttack")
		{
			player_atk_cpnt = &transform;
		}
		else if (transform.name.find("Cage") != std::string::npos)
		{
			Cage cage;
			cage.index = cage_index;
			cage.transform = &transform;
			cages.emplace_back(cage);
			cage_index++;
		}
		else if (transform.name == "Boots")
		{
			boots.transform = &transform;
			cages.begin()->item = &boots;
		}
		else if (transform.name == "ComponentBoots")
		{
			component_boots = &transform;
			boots_scale = component_boots->scale;
			component_boots->scale = glm::vec4(0);
		}
		else if (transform.name == "ComponentWings")
		{
			component_wings = &transform;
			wings_scale = component_wings->scale;
			component_wings->scale = glm::vec4(0);
		}
		else if (transform.name.find("Collider") != std::string::npos)
		{
			Platform platform;
			platform.transform = &transform;
			platform.name = transform.name;
			platform.pos = transform.position;
			// platform.pos = transform.make_local_to_world() * glm::vec4(transform.position, 1.0f);
			platform.width = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).x - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).x);
			platform.height = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).z - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).z);
			platform.fragile = false;
			platform.visible = true;
			platform.stepping_time = 0.0f;
			platforms.emplace_back(platform);
		}
		else if (transform.name.find("Fragile") != std::string::npos)
		{
			Platform platform;
			platform.transform = &transform;
			platform.name = transform.name;
			platform.pos = transform.position;
			// platform.pos = transform.make_local_to_world() * glm::vec4(transform.position, 1.0f);
			platform.width = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).x - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).x);
			platform.height = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).z - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).z);
			platform.fragile = true;
			platform.visible = true;
			platform.stepping_time = 0.0f;
			platforms.emplace_back(platform);
			if (transform.name == "Fragile5")
			{
				fragile5 = &transform;
			}
		}
		else if (transform.name.find("Spike") != std::string::npos)
		{
			Spike spike;
			spike.pos = transform.make_local_to_world() * glm::vec4(transform.position, 1.0f);
			spike.width = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).x - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).x);
			spike.height = (float)abs((transform.make_local_to_world() * glm::vec4(transform.max, 1.0f)).z - (transform.make_local_to_world() * glm::vec4(transform.min, 1.0f)).z);
			spikes.emplace_back(spike);
		}
		else if (transform.name == "Final_Boss")
		{
			final_boss.transform = &transform;
			final_boss.hasweapon = false;
			final_boss.speed = 0.01f;
		}
		else if (transform.name.find("FinalTeleport") != std::string::npos)
		{
			final_teleportPos.emplace_back(transform.position);
		}
		else if (transform.name == "Level01Teleport")
		{
			level1_tel = &transform;
		}
		else if (transform.name.find("Enemy") != std::string::npos)
		{
			Enemy enemy;
			enemy.index = (int)enemies.size();
			enemy.transform = &transform;
			enemy.stepped_plat = fragile5;
			// enemy.canmove = false;
			enemies.emplace_back(enemy);
			current_enemy = &enemy;
		}
		else if (transform.name.find("EnemSub") != std::string::npos)
		{
			transform.scale = glm::vec3(0.0f);
			if (enemy_subuv_count < 18)
				enemy_subuv[0].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 18 && enemy_subuv_count < 36)
				enemy_subuv[1].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 36 && enemy_subuv_count < 54)
				enemy_subuv[2].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 54 && enemy_subuv_count < 72)
				enemy_subuv[3].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 72 && enemy_subuv_count < 90)
				enemy_subuv[4].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 90 && enemy_subuv_count < 108)
				enemy_subuv[5].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 108 && enemy_subuv_count < 126)
				enemy_subuv[6].subtransforms.emplace_back(&transform);
			else if (enemy_subuv_count >= 126 && enemy_subuv_count < 144)
				enemy_subuv[7].subtransforms.emplace_back(&transform);
			else
				enemy_subuv[8].subtransforms.emplace_back(&transform);
			enemy_subuv_count++;
		}
		else if (transform.name == "Wing")
		{
			wings = &transform;
		}
		else if (transform.name.find("Star") != std::string::npos)
		{
			Item *star = new Item();
			star->transform = &transform;
			auto bi = cages.begin();
			std::advance(bi, star_idx);
			bi->item = star;
			star_idx++;
		}
		else if (transform.name.find("FinalMoveTarget") != std::string::npos)
		{
			final_move_pos.emplace_back(transform.position);
		}
		else if (transform.name.find("Boom") != std::string::npos)
		{
			Boom *boom = new Boom();
			boom->transform = &transform;
			boom->ori_scale = transform.scale;
			booms.emplace_back(*boom);
		}
		else if (transform.name.find("Explode") != std::string::npos)
		{
			auto p = booms.begin();
			std::advance(p, explode_idx);
			p->explode = &transform;
			explode_idx++;
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

	change_rand_pos();

	// get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	camera->transform->position = glm::vec3{player->position.x + 2.5722f, -30.0f, player->position.z + 1.3373f};

	// start music loop playing:
	//  (note: position will be over-ridden in update())
	music = Sound::loop(*bgm_sample, 0.3f);
	boss1_loop_sound = Sound::loop_3D(*voice_06_sample, 2.0f, level1_boss.transform->position, 0.25f);
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (enter.pressed)
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
			else if (evt.key.keysym.sym == SDLK_SPACE)
			{
				space.downs += 1;
				space.pressed = true;
				if (first_jump == false)
					player_status = PlayerStatus::JumpStart;
				if (!first_jump && !hasJetPack && !player_die)
					Sound::play_3D(*sound_02_sample, 0.75f, player->position);
				else if (!first_jump && hasJetPack && !player_die)
					Sound::play_3D(*sound_03_sample, 1.0f, player->position);
				else if (hasWings)
					beatWings = true;
				return true;
			}
		}

		if (evt.key.keysym.sym == SDLK_RETURN)
		{
			enter.pressed = true;
			start_menu->scale = glm::vec3(0);
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
		else if (evt.key.keysym.sym == SDLK_SPACE)
		{
			space.pressed = false;
			beatWings = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		keyatk.downs += 1;
		keyatk.pressed = true;
		if (get_weapon)
			weapon_status = WeaponStatus::NormalAttack;
		return true;
	}
	else if (evt.type == SDL_MOUSEBUTTONUP)
	{
		keyatk.pressed = false;
		attack = false;
		return true;
	}
	return false;
}

void PlayMode::update(float elapsed)
{
	if (!game_end)
	{

		update_player_status();
		if (get_weapon)
			update_weapon_status();
		if (hasWings)
			update_wings_status();

		for (auto enemy : enemies)
			update_enemy_status(enemy);

		// get weapon
		if (!get_weapon && hit_detect_SAT(player, player_atk_cpnt).overlapped)
		{
			get_weapon = true;
			component->scale = component_scale;
			player_atk_cpnt->scale = glm::vec4(0);
			sound = Sound::play_3D(*sound_04_sample, 1.0f, player->position);
		}

		// hit cage
		for (auto &cage : cages)
		{
			// hit cage
			if (get_weapon && !cage.isDestroied && keyatk.pressed && !attack && hit_detect_SAT(component, cage.transform).overlapped)
			{
				attack = true;
				cage.isDestroied = true;
				cage.transform->scale = glm::vec4(0);
				sound = Sound::play_3D(*sound_01_sample, 1.0f, cage.transform->position);
			}
			// get item
			if (!cage.item->has && cage.isDestroied && hit_detect(player, cage.item->transform).overlapped)
			{
				cage.item->has = true;
				cage.item->transform->scale = glm::vec4(0);
				// get boots
				if (cage.item->transform->name == boots.transform->name)
				{
					if (!hasJetPack)
					{
						Sound::play_3D(*voice_02_sample, 1.0f, cages.begin()->transform->position);
						sound = Sound::play_3D(*voice_01_sample, 1.5f, player->position);
					}
				}
				else
				{
					// get star
					star_count += 1;
					Sound::play_3D(*sound_04_sample, 1.0f, player->position);
				}
			}
		}

		// get wings
		if (!hasWings && hit_detect_SAT(player, wings).overlapped)
		{
			wings->scale = glm::vec3(0);
			hasWings = true;
			hasJetPack = false;
			Sound::play_3D(*sound_07_sample, 1.0f, player->position);
			Sound::play_3D(*voice_04_sample, 2.0f, player->position);
		}

		if (second_jump && boots.has)
		{
			boots_timer = std::min(1.0f, boots_timer + elapsed * 5);
			component_boots->scale = boots_scale * boots_timer;
		}

		if (beatWings)
		{
			wings_timer = std::min(1.0f, wings_timer + elapsed * 15.0f);
			component_wings->scale = wings_scale * wings_timer;
		}

		if (!beatWings)
		{
			if (wings_timer > 0.0f)
			{
				wings_timer = std::min(1.0f, wings_timer - elapsed * 1.5f);
				component_wings->scale = wings_scale * wings_timer;
			}
			else
				wings_timer = 0.0f;
		}

		if (invincible)
		{
			invincible_time += elapsed;
		}
		if (invincible_time > 1.0f)
		{
			invincible = false;
			invincible_time = 0.0f;
		}

		// player die
		if (player_hp->scale.x <= 0.0001f)
		{
			player->scale = glm::vec3(0);
			player_die = true;
			if (revive_time >= 3.23f)
				Sound::play_3D(*voice_05_sample, 1.25f, player->position);
		}

		// boss die
		if (current_boss->current_hp <= 0.0001f)
		{
			// current_boss->transform->scale = glm::vec3(0);
			for (auto &bullet : current_bullets)
			{
				put_away_bullet(bullet);
			}
			// get jet pack and replace boots
			if (!hasJetPack && player_stage == PlayerStage::InitialStage)
			{
				hasJetPack = true;
				boots.has = false;
				// boss1 death roar
				Sound::play_3D(*voice_07_sample, 2.0f, level1_boss.transform->position);
				// has jet pack
				Sound::play_3D(*voice_03_sample, 2.0f, player->position);
			}

			if (!current_boss->die)
			{
				current_boss->die = level1_boss_dead();
				current_boss->status = Dead;
				boss_hp_bg->scale = glm::vec3(0);
				(*boss1_loop_sound).stop();
			}

			if (current_boss->hasweapon)
				current_boss->weapon->transform->scale = glm::vec3(0);

			// game end
			if (final_boss.die)
			{
				game_end = true;
			}
		}
		else
		{
			// bullet attack
			for (auto &bullet : current_bullets)
			{

				if (!bullet.hit_player && hit_detect(player, bullet.transform).overlapped)
				{
					hit_player(0.05f);
					bullet.hit_player = true;
					put_away_bullet(bullet);
				}
			}
			// boss weapon attack
			if (current_boss->hasweapon && current_boss->weapon->transform->scale.x > 0.00001f)
			{
				if (hit_detect_SAT(player, current_boss->weapon->transform).overlapped)
				{
					hit_player(0.05f);
				}
			}
			else
			{
				if (hit_detect_SAT(player, current_boss->transform).overlapped)
				{
					hit_player(0.05f);
				}
			}

			// boom explode
			for (auto &boom : booms)
			{
				if (boom.ready_explode)
				{
					if (boom.explode_countdown > 3)
					{
						boom.transform->scale = glm::vec3(0);
						boom.explode->scale = boom.ori_scale;
						boom.explode->position = boom.transform->position;
						boom.explode_countdown = 0;
						boom.ready_explode = false;
						boom.start_explode = true;
					}
					else
					{
						boom.explode_countdown += elapsed;
					}
				}
				if (boom.start_explode)
				{
					// explode hit
					if (hit_detect_SAT(player, boom.explode).overlapped)
					{
						hit_player(0.05f);
					}
					// play explode ani
					play_explode_ani(&boom);
				}
			}

			// update boss status
			update_boss_status();

			// boss status
			switch (current_boss->status)
			{
			case Idle:
			{
				boss_hp_bg->scale = glm::vec3(0);
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

				if (current_boss->hasweapon)
				{
					current_boss->weapon->transform->scale = glm::vec3(0);
				}
				break;
			}
			case Weak:
			{
				// show hp
				boss_hp_bg->scale = glm::vec3(1.66f, 0.10048f, 0.3f);
				boss_hp->scale.x = current_boss->current_hp / current_boss->max_hp * ori_bosshp_scale.x;
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
				// hide weapon
				if (current_boss->hasweapon)
				{
					current_boss->weapon->transform->scale = glm::vec3(0);
				}
				break;
			}
			case Melee:
			{
				// start weak timer
				start_weak_timer = true;

				// show hp
				boss_hp_bg->scale = glm::vec3(1.66f, 0.10048f, 0.3f);
				boss_hp->scale.x = current_boss->current_hp / current_boss->max_hp * ori_bosshp_scale.x;

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
				if (current_boss->transform->name == level1_boss.transform->name)
				{
					glm::vec3 dir = glm::normalize(player->position - current_boss->transform->position);
					(*boss1_loop_sound).position = current_boss->transform->position;
					float vec = enemy_gravity * elapsed;
					if (current_boss->hasweapon)
					{
						// show weapon
						current_boss->weapon->transform->scale = current_boss->weapon->ori_weap_scale;
						if (!hit_detect_SAT(player, current_boss->weapon->transform).overlapped)
						{
							glm::vec3 expected_pos = glm::vec3(current_boss->transform->position.x + dir.x * current_boss->speed * elapsed, current_boss->transform->position.y, current_boss->transform->position.z + vec);
							current_boss->transform->position = enemy_land_on_platform(current_boss->transform, expected_pos);
						}
					}
					else
					{
						if (!hit_detect_SAT(player, current_boss->transform).overlapped)
						{
							glm::vec3 expected_pos = glm::vec3(current_boss->transform->position.x + dir.x * current_boss->speed * elapsed, current_boss->transform->position.y, current_boss->transform->position.z + vec);
							current_boss->transform->position = enemy_land_on_platform(current_boss->transform, expected_pos);
						}
					}
				}
				else if (current_boss->transform->name == final_boss.transform->name)
				{
					// random move
					if (glm::distance(current_boss->transform->position, rand_pos) > 0.1f)
					{
						// place boom
						if (place_boom_timer > 1)
						{
							place_boom_timer = 0;
							// std::cout << "boom_count: " << boom_count << std::endl;
							if (boom_count < 9)
							{
								boom_count++;
								auto p = booms.begin();
								std::advance(p, last_boom_idx);
								p->transform->scale = p->ori_scale;
								p->transform->position = current_boss->transform->position;
								p->ready_explode = true;
								// std::cout << p->transform->name << " ready explode set true!!" << std::endl;
								if (last_boom_idx < 8)
									last_boom_idx++;
								else
									last_boom_idx = 0;
							}
							else
							{
								// reach max, stop place
							}
						}
						place_boom_timer += elapsed;

						// random move
						rand_pos_time += current_boss->speed * elapsed;
						current_boss->transform->position = bullet_current_Pos(current_boss->transform->position, rand_pos, rand_pos_time);
					}
					// change random pos
					else
					{
						rand_pos_time = 0;
						change_rand_pos();
					}
				}

				break;
			}
			case Shoot:
			{
				// start weak timer
				start_weak_timer = true;

				// show hp
				boss_hp_bg->scale = glm::vec3(1.66f, 0.10048f, 0.3f);
				boss_hp->scale.x = current_boss->current_hp / current_boss->max_hp * ori_bosshp_scale.x;
				// show weapon
				if (current_boss->hasweapon)
					current_boss->weapon->transform->scale = current_boss->weapon->ori_weap_scale;
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
				}

				break;
			}
			case Dead: // won't go this case
			{
				boss_hp_bg->scale = glm::vec3(0);
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

		// enemy
		for (auto &enemy : enemies)
		{
			// enemy die
			if (enemy.current_hp <= 0.00001f)
			{
				enemy.status = EnemyStatus::Dead;
				// enemy.transform->scale = glm::vec3(0);
				enemy_dead(enemy);
			}
			else
			{
				// player attack
				if (get_weapon &&
					keyatk.pressed &&
					!attack && hit_detect_SAT(component, enemy.transform).overlapped)
				{
					enemy.status = EnemyStatus::Damaged;
					attack = true;
					enemy.current_hp -= 0.2f;
				}

				if (enemy.status == EnemyStatus::Damaged)
					enemy.damage_time += elapsed;

				// hit player
				if (hit_detect_SAT(player, enemy.transform).overlapped)
				{
					hit_player(0.05f);
				}

				// // move
				if (enemy.canmove && (enemy.status != EnemyStatus::Damaged || enemy.damage_time > 0.4f))
				{
					enemy.status = EnemyStatus::Move;
					current_enemy = &enemy;
					float expectedX = enemy.transform->position.x + enemy.speed * elapsed;
					float vec = enemy.transform->position.z + enemy_gravity * elapsed;
					glm::vec3 expected_pos = glm::vec3(expectedX, enemy.transform->position.y, vec);
					enemy.damage_time = 0.0f;

					enemy_land_on_platform(enemy.transform, expected_pos);
					if (enemy.stepped_plat && enemy.stepped_plat->name != "Fragile5")
					{
						if (expectedX > (enemy.stepped_plat->make_local_to_world() * glm::vec4(enemy.stepped_plat->max, 1.0f)).x - 0.2f)
						{
							enemy.speed *= -1;
							enemy.transform->scale.x *= -1;
							expectedX = (enemy.stepped_plat->make_local_to_world() * glm::vec4(enemy.stepped_plat->max, 1.0f)).x - 0.2f;
						}
						else if (expectedX < (enemy.stepped_plat->make_local_to_world() * glm::vec4(enemy.stepped_plat->min, 1.0f)).x + 0.2f)
						{
							enemy.speed *= -1;
							enemy.transform->scale.x *= -1;
							expectedX = (enemy.stepped_plat->make_local_to_world() * glm::vec4(enemy.stepped_plat->min, 1.0f)).x + 0.2f;
						}

						expected_pos = glm::vec3(expectedX, enemy.transform->position.y, vec);
					}
					enemy.transform->position = enemy_land_on_platform(enemy.transform, expected_pos);
				}
				else if (enemy.status != EnemyStatus::Damaged || enemy.damage_time > 0.4f)
				{
					// stationary enemy
					enemy.status = EnemyStatus::Idle;
					enemy.damage_time = 0.0f;
				}
			}
		}

		// player attack
		if (get_weapon &&
			keyatk.pressed &&
			!attack && hit_detect_SAT(component, current_boss->transform).overlapped)
		{
			attack = true;
			hit_boss(0.1f);

			// count for teleport
			if (current_boss->transform->name == final_boss.transform->name)
			{
				if (count_for_teleport > 2)
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
				else if (first_jump && !second_jump && jump_signal && boots.has && !hasJetPack && !player_die)
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
					// hover_max_time = hover_full_fuel_time / jetpack_max_fuel * jetpack_fuel;
				}
				else if (hasWings)
				{
					if (wings_energy >= 0)
					{
						jump_velocity = 2.2f;
						flying = true;
					}
				}
			}

			if (!space.pressed)
			{
				jump_signal = true;
				jetpack_on = false;
			}

			if (hasJetPack && jetpack_fuel > 0 && jetpack_on && !player_die)
			{
				jetpack_fuel -= elapsed;
			}

			if (hasWings && jetpack_fuel > 0 && flying && !player_die)
			{
				wings_energy -= elapsed;
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

			// useless, just for enemy platform detect
			Scene::Transform stepped_plat;
			if (on_platform(player, stepped_plat))
			{
				first_jump = false;
				second_jump = false;
				jump_velocity = 0;
				// hover_time = 0;
				jump_signal = false;

				jetpack_on = false;
				flying = false;
				if (hasJetPack && jetpack_fuel < jetpack_max_fuel)
				{
					jetpack_fuel += 2 * elapsed;
				}
				if (hasWings && wings_energy < wings_max_energy)
				{
					wings_energy += 2 * elapsed;
				}

				// hurt based on max fall speed
				if (max_fall_speed < 0)
				{
					std::cout << max_fall_speed << "\n";
				}
				if (max_fall_speed <= -7.0f)
				{
					if (max_fall_speed <= -12.0f)
					{
						hit_player(0.2f);
					}
					else
					{
						float damage = 0.1f + (-7.0f - max_fall_speed) / 5.0f * 0.1f;
						hit_player(damage);
					}
				}
				max_fall_speed = 0;
			}

			on_platform_step(elapsed);
			hit_spike();

			if (!hasJetPack && !hasWings)
			{
				player_fuel->scale.x = 0.001f;
			}
			else if (hasJetPack)
			{
				if (jetpack_fuel <= 0)
				{
					player_fuel->scale.x = 0.001f;
				}
				else if (jetpack_fuel >= jetpack_max_fuel)
				{
					player_fuel->scale.x = max_fuel_scale;
				}
				else
				{
					player_fuel->scale.x = max_fuel_scale * jetpack_fuel / jetpack_max_fuel;
				}
			}
			else if (hasWings)
			{
				if (wings_energy <= 0)
				{
					player_fuel->scale.x = 0.001f;
				}
				else if (wings_energy >= wings_max_energy)
				{
					player_fuel->scale.x = max_fuel_scale;
				}
				else
				{
					player_fuel->scale.x = max_fuel_scale * wings_energy / wings_max_energy;
				}
			}

			if (hit_platform())
			{
				jump_velocity = 0;
			}

			if (stage_changing)
			{
				hori_move = 0;
				if (vert_move > 0)
					vert_move = 0;
				if (jump_velocity > 0)
					jump_velocity = 0;
				stage_change_timer += elapsed;
				if (stage_change_timer > 1.0f)
				{
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
			// std::cout << player->position.x << "," << player->position.z << "\n";
			check_dropping();
			revive(elapsed);

			// update max fall speed
			if (jump_velocity <= max_fall_speed)
			{
				max_fall_speed = jump_velocity;
			}
		}

		{ // update listener to camera position:
			// glm::mat4x3 frame = camera->transform->make_local_to_parent();
			glm::mat4x3 frame = player->make_local_to_parent();
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

	{
		if (enter.pressed)
		{
			text.Draw(text_program, "A & D to move, Space to jump, Left Click to attack", -72.0f, 20.0f, drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.4f);
			text.Draw(text_program, std::to_string(star_count), drawable_size.x / 2.0f - (635.0f * drawable_size.y / 720.0f) + 0.12f * (drawable_size.y - 720.0f), drawable_size.y - (125.0f * drawable_size.y / 720.0f), drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.25f * drawable_size.y / 720.0f);
			text.Draw(text_program, "  / 7", drawable_size.x / 2.0f - (635.0f * drawable_size.y / 720.0f) + 0.12f * (drawable_size.y - 720.0f), drawable_size.y - (125.0f * drawable_size.y / 720.0f), drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.25f * drawable_size.y / 720.0f);
			text.Draw(text_program, "Death: ", drawable_size.x / 2.0f - (675.0f * drawable_size.y / 720.0f) + 0.12f * (drawable_size.y - 720.0f), drawable_size.y - (155.0f * drawable_size.y / 720.0f), drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.25f * drawable_size.y / 720.0f);
			text.Draw(text_program, std::to_string(death_time), drawable_size.x / 2.0f - (585.0f * drawable_size.y / 720.0f) + 0.12f * (drawable_size.y - 720.0f), drawable_size.y - (155.0f * drawable_size.y / 720.0f), drawable_size, glm::vec3(1.0f, 1.0f, 1.0f), 0.25f * drawable_size.y / 720.0f);
		}
		if (final_boss.die)
		{
			glDepthFunc(GL_LEQUAL);
			text.Draw(text_program, "Congrats! You Win!", drawable_size.x / 2.0f - (540.0f * drawable_size.y / 720.0f), 330.0f * drawable_size.y / 720.0f, drawable_size, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f * drawable_size.y / 720.0f);
			text.Draw(text_program, "Congrats! You Win!", drawable_size.x / 2.0f - (535.0f * drawable_size.y / 720.0f), 335.0f * drawable_size.y / 720.0f, drawable_size, glm::vec3(0.25f, 0.95f, 0.75f), 1.0f * drawable_size.y / 720.0f);
			glDepthFunc(GL_LESS);
		}
	}

	GL_ERRORS();
}

glm::vec3 world_coords(Scene::Transform *block)
{
	auto world_block = block->make_local_to_world() * glm::vec4(block->position, 1.0f);
	return world_block;
}

glm::vec3 PlayMode::check_change_stage(glm::vec3 expected_position)
{
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
			boss_hp_bg->scale = glm::vec3(0);
		}
	}

	if (player_stage == PlayerStage::JumpGame && hasWings)
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

			current_boss = &final_boss;
			current_boss->hasweapon = false;
			current_boss->status = Idle;
			current_boss->current_hp = 1.0f;

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

	if (invincible)
	{
		subuv.start_index = 36;
		subuv.range = 3;
		subuv.speed = 1.2f;
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

	if (bit == 2 && weapon_subuv.anim_timer < 0.4f && !player_die)
		Sound::play_3D(*sound_05_sample, 1.0f, player->position);

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

void PlayMode::update_wings_status()
{
	wings_subuv.anim_timer += 0.1f * wings_subuv.speed;

	uint32_t bit = 1;
	while (true)
	{
		if (wings_subuv.bitmask >> bit == 0)
			break;
		bit++;
	}

	wings_subuv.start_index = 0;
	wings_subuv.range = 3;
	wings_subuv.speed = 2.0f;

	if (wings_subuv.anim_timer > 1)
	{
		wings_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit++;
		if (bit - 1 >= wings_subuv.start_index + wings_subuv.range)
		{
			bit = wings_subuv.start_index + 1;
		}
		wings_subuv.bitmask = 1ULL << (bit - 1);
		wings_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
		wings_subuv.anim_timer = 0.0f;
	}
}

void PlayMode::hit_player(float damage)
{
	if (!invincible)
	{
		if (!player_die)
			Sound::play_3D(*sound_06_sample, 1.0f, player->position);

		if (player_hp->scale.x > 0.0001f)
		{
			player_hp->scale.x -= max_player_hp * damage;
		}
		invincible = true;
	}
}

void PlayMode::hit_spike()
{
	for (auto &spike : spikes)
	{
		if (((std::abs(player->position.z - spike.pos.z)) <= spike.height / 2) && ((std::abs(player->position.x - spike.pos.x)) <= spike.width / 2))
		{
			hit_player(0.05f);
		}
	}
}

void PlayMode::hit_boss(float damage)
{
	if (current_boss->current_hp > 0.0001f)
	{
		current_boss->current_hp -= damage;
		boss_hp->scale.x = current_boss->current_hp / current_boss->max_hp * ori_bosshp_scale.x;

		current_boss->status = BattleStatus::Attacked;
	}
}

bool PlayMode::on_platform(Scene::Transform *obj, Scene::Transform &stepped_platform)
{
	for (auto &platform : platforms)
	{
		if (platform.visible)
		{
			if (obj->position.z == platform.pos.z + platform.height / 2)
			{
				stepped_platform = *(platform.transform);
				if (!platform.fragile)
				{
					return true;
				}
				if (platform.fragile && platform.visible)
				{
					return true;
				}
			}
		}
	}
	if (obj->name.find("Enemy") != std::string::npos)
	{
		obj->scale = glm::vec3(0);
		return false;
	}
	else
	{
		return obj->position.z == start_point.z;
	}
}

void PlayMode::on_platform_step(float elapsed)
{
	for (auto &platform : platforms)
	{
		if (platform.visible)
		{
			if (player->position.z == platform.pos.z + platform.height / 2)
			{
				if (platform.fragile && platform.stepping_time < 0.5f)
				{
					platform.stepping_time += elapsed;
				}
				if (platform.fragile && platform.stepping_time >= 0.5f)
				{
					platform.visible = false;
					platform.transform->scale.x = 0.0f;
					platform.transform->scale.y = 0.0f;
					platform.transform->scale.z = 0.0f;
				}
			}
		}
	}
}

bool PlayMode::hit_platform()
{
	for (auto platform : platforms)
	{
		if (platform.visible)
		{
			if (player->position.z == platform.pos.z - platform.height / 2)
			{
				return true;
			}
		}
	}
	return false;
}

void PlayMode::land_on_platform(glm::vec3 expected_position)
{
	for (auto platform : platforms)
	{
		if (platform.visible)
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
	}
	// camera->transform->position += move.x * frame_right + move.y * frame_forward;
	// player->position += move.x * frame_right + move.y * frame_forward + move.z * frame_up;

	// if (expected_position.z <= start_point.z && std::abs(expected_position.x - (-11.5f)) < 4.4f)
	// // hardcode to prevent a jump at start
	// {
	//   expected_position.z = start_point.z;
	// }
	// if (expected_position.z < start_point.z)
	// // hardcode to prevent a jump at start
	// {
	// 	expected_position.z = start_point.z;
	// }
	if (!player_die)
	{
		camera->transform->position += expected_position - player->position;
		player->position = expected_position;
	}
}

glm::vec3 PlayMode::enemy_land_on_platform(Scene::Transform *enemy, glm::vec3 exp_position)
{
	glm::vec3 expected_pos = exp_position;
	for (auto platform : platforms)
	{
		if (platform.visible)
		{
			// std::cout << "name:" << platform.name << ",expected pos:" << expected_position.z << ",plat pos:" << platform.pos.z << ",3:" << std::abs(expected_position.z - platform.pos.z) << "4:" << platform.height / 2 << std::endl;
			if (std::abs(exp_position.x - platform.pos.x) < platform.width / 2 &&
				std::abs(exp_position.z - platform.pos.z) < platform.height / 2)
			{

				// This is real collision detection
				if (enemy->position.z >= platform.pos.z + platform.height / 2)
				{
					if (exp_position.z < platform.pos.z + platform.height / 2)
					{
						// from higher position
						Scene::Transform *trans;
						trans = platform.transform;
						current_enemy->stepped_plat = trans;
						expected_pos.z = platform.pos.z + platform.height / 2;
					}
				}
				if (enemy->position.z <= platform.pos.z - platform.height / 2)
				{
					if (exp_position.z > platform.pos.z - platform.height / 2)
					{
						// for lower position
						expected_pos.z = platform.pos.z - platform.height / 2;
					}
					if (enemy->position.x < platform.pos.x - platform.width / 2 &&
						exp_position.x > platform.pos.x - platform.width / 2)
					{

						expected_pos.x = platform.pos.x - platform.width / 2;
					}
					if (enemy->position.x > platform.pos.x + platform.width / 2 &&
						exp_position.x < platform.pos.x + platform.width / 2)
					{

						expected_pos.x = platform.pos.x + platform.width / 2;
					}
				}
				else
				{
					if (enemy->position.x <= platform.pos.x - platform.width / 2)
					{
						if (exp_position.x > platform.pos.x - platform.width / 2)
						{

							expected_pos.x = platform.pos.x - platform.width / 2;
							if (current_boss->status != Dead && current_boss->transform->name == "Boss" && enemy->name == "Boss")
								ready_to_teleport = true;
						}
					}
					if (enemy->position.x >= platform.pos.x + platform.width / 2)
					{
						if (exp_position.x < platform.pos.x + platform.width / 2)
						{
							expected_pos.x = platform.pos.x + platform.width / 2;
						}
					}
				}
			}
		}
	}
	expected_pos.y = exp_position.y;
	return expected_pos;
}

void PlayMode::check_dropping()
{
	if (player_stage == PlayerStage::InitialStage)
	{
		if (player->position.z < 0.5f)
		{
			player_die = true;
		}
	}
	else if (player_stage == PlayerStage::JumpGame)
	{
		if (player->position.z < 0.5f)
		{
			player_die = true;
		}
	}
}

void PlayMode::revive(float elapsed)
{
	if (player_stage == PlayerStage::InitialStage)
	{
		if (player_die && !waiting_revive)
		{
			waiting_revive = true;
			player->scale = glm::vec3(0, 0, 0);
			player_hp->scale.x = 0;
		}
		if (waiting_revive && revive_time > 0)
		{
			revive_time -= elapsed;
		}
		if (revive_time <= 0)
		{
			if (player_die)
			{
				death_time += 1;
			}
			player_die = false;
			waiting_revive = false;
			player->scale = player_origin_scale;
			camera->transform->position.x += 2.2f - player->position.x;
			camera->transform->position.z += 1.2f - player->position.z;
			player->position.x = 2.2f;
			player->position.z = 1.2f;
			player_hp->scale.x = max_player_hp;
			revive_time = revive_max_time;
			jump_velocity = 0;
			max_fall_speed = 0;
		}
	}
	if (player_stage == PlayerStage::JumpGame)
	{
		if (player_die && !waiting_revive)
		{
			waiting_revive = true;
			player->scale = glm::vec3(0, 0, 0);
			player_hp->scale.x = 0;
		}
		if (waiting_revive && revive_time > 0)
		{
			revive_time -= elapsed;
		}
		if (revive_time <= 0)
		{
			if (player_die)
			{
				death_time += 1;
			}
			player_die = false;
			waiting_revive = false;
			player->scale = player_origin_scale;
			camera->transform->position.x += 38.0f - player->position.x;
			camera->transform->position.z += 5.3f - player->position.z;
			player->position.x = 38.0f;
			player->position.z = 5.3f;
			player_hp->scale.x = max_player_hp;
			revive_time = revive_max_time;
			jump_velocity = 0;
			max_fall_speed = 0;
		}
	}
	if (player_stage == PlayerStage::BossTeleport)
	{
		if (player_die && !waiting_revive)
		{
			waiting_revive = true;
			player->scale = glm::vec3(0, 0, 0);
			player_hp->scale.x = 0;
		}
		if (waiting_revive && revive_time > 0)
		{
			revive_time -= elapsed;
		}
		if (revive_time <= 0)
		{
			if (player_die)
			{
				death_time += 1;
			}
			player_die = false;
			waiting_revive = false;
			player->scale = player_origin_scale;
			camera->transform->position.x += 63.0f - player->position.x;
			camera->transform->position.z += 5.0f - player->position.z;
			player->position.x = 63.0f;
			player->position.z = 5.0f;
			player_hp->scale.x = max_player_hp;
			revive_time = revive_max_time;
			jump_velocity = 0;
			max_fall_speed = 0;
		}
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

	for (auto &telepos : final_teleportPos)
	{

		float dis = glm::distance(player->position, telepos);
		minpos = dis < mindis ? telepos : minpos;
		mindis = dis < mindis ? dis : mindis;
	}
	return minpos;
}
void PlayMode::update_boss_status()
{
	{ // Update boss animation
		if (current_boss->status != Weak)
		{
			if (current_boss->transform->name == level1_boss.transform->name)
			{
				if (player->position.x > current_boss->transform->position.x)
					current_boss->transform->scale.x = -0.424f;
				else
					current_boss->transform->scale.x = 0.424f;
			}
		}

		boss_subuv.anim_timer += 0.1f * boss_subuv.speed;

		uint32_t bit = 1;
		while (true)
		{
			if (boss_subuv.bitmask >> bit == 0)
				break;
			bit++;
		}

		switch (current_boss->status)
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

		if (current_boss->status == BattleStatus::Dead)
			return;
		else if (current_boss->status == BattleStatus::Attacked)
		{
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit = 11;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			attacked_timer += 0.1f;
			if (attacked_timer > 1.25f)
			{
				current_boss->status = BattleStatus::Weak;
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

	// idle(hide boss weapon) every 3s,only for level1 boss
	if (start_weak_timer)
	{
		if (current_boss->transform->name == level1_boss.transform->name)
		{
			weak_timer++;
			// std::cout << "time:" << weak_timer << std::endl;
			if (weak_timer > 500 && detect_boss_status)
			{
				current_boss->status = Weak;
				detect_boss_status = false;
				weak_timer = 0;
			}
			if (weak_timer > 250 && !detect_boss_status)
			{
				detect_boss_status = true;
				weak_timer = 0;
			}
		}
	}

	if (detect_boss_status)
	{
		if (current_boss->transform->name == final_boss.transform->name)
		{
			if (rand_move_timer > 600 && current_boss->status == Melee)
			{
				current_boss->status = Weak;
				rand_move_timer = 0;
			}
			else if (rand_move_timer > 200 && current_boss->status != Melee)
			{
				current_boss->status = Melee;
				rand_move_timer = 0;
			}
			else
			{
				rand_move_timer++;
			}

			if (current_boss->status != BattleStatus::Attacked && current_boss->status != BattleStatus::Dead && glm::distance(player->position, current_boss->transform->position) > 15.0f)
			{
				current_boss->status = Idle;
			}
			else
			{
				// active first melee
				if (!first_melee)
				{
					current_boss->status = Melee;
					first_melee = true;
				}
			}
			// if (glm::distance(player->position, current_boss->transform->position) < 2.5f && current_boss->status != BattleStatus::Attacked && current_boss->status != Dead)
			// {
			// 	current_boss->status = Melee;
			// }
			// else if (glm::distance(player->position, current_boss->transform->position) < 8.0f && current_boss->status != BattleStatus::Attacked && current_boss->status != Dead)
			// {
			// 	current_boss->status = Shoot;
			// }
		}
		else
		{
			if (glm::distance(player->position, current_boss->transform->position) < 2.5f && current_boss->status != BattleStatus::Attacked && current_boss->status != Dead)
			{

				current_boss->status = Melee;
			}
			else if (glm::distance(player->position, current_boss->transform->position) < 5.0f && current_boss->status != BattleStatus::Attacked && current_boss->status != Dead)
			{
				current_boss->status = Shoot;
			}
			else if (current_boss->status != BattleStatus::Attacked && glm::distance(player->position, current_boss->transform->position) > 5.0f)
			{
				// current_boss->status = BattleStatus::Idle;
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
			if (bit == 11)
				bit = 7;
			else
				bit++;
			boss_subuv.bitmask = 1ULL << (bit - 1);
			boss_subuv.subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			boss_subuv.anim_timer = 0.0f;
		}

		return false;
	}
}

void PlayMode::update_enemy_status(Enemy enemy)
{
	enemy_subuv[enemy.index].anim_timer += 0.1f * enemy_subuv[enemy.index].speed;

	uint32_t bit = 1;
	while (true)
	{
		if (enemy_subuv[enemy.index].bitmask >> bit == 0)
			break;
		bit++;
	}

	switch (enemy.status)
	{
	case EnemyStatus::Damaged:
		enemy_subuv[enemy.index].start_index = 4;
		enemy_subuv[enemy.index].range = 4;
		enemy_subuv[enemy.index].speed = 1.0f;
		break;
	default:
		enemy_subuv[enemy.index].start_index = 0;
		enemy_subuv[enemy.index].range = 3;
		enemy_subuv[enemy.index].speed = 1.25f;
	}

	if (enemy.status == EnemyStatus::Dead)
		return;
	// else if (enemy.status == EnemyStatus::Damaged)
	// {
	// 	enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(0.0f);
	// 	bit = 11;
	// 	enemy_subuv[enemy.index].bitmask = 1ULL << (bit - 1);
	// 	enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(1.0f);
	// 	attacked_timer += 0.1f;
	// 	if (attacked_timer > 1.25f)
	// 	{
	// 		enemy.status = BattleStatus::Idle;
	// 		attacked_timer = 0.0f;
	// 	}
	// }
	else if (enemy_subuv[enemy.index].anim_timer > 1)
	{
		enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit++;
		if (bit - 1 >= enemy_subuv[enemy.index].start_index + enemy_subuv[enemy.index].range)
			bit = enemy_subuv[enemy.index].start_index + 1;
		enemy_subuv[enemy.index].bitmask = 1ULL << (bit - 1);
		enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(1.0f);
		enemy_subuv[enemy.index].anim_timer = 0.0f;
	}
	else if (bit - 1 >= enemy_subuv[enemy.index].start_index + enemy_subuv[enemy.index].range || bit - 1 < enemy_subuv[enemy.index].start_index)
	{
		enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(0.0f);
		bit = enemy_subuv[enemy.index].start_index + 1;
		enemy_subuv[enemy.index].bitmask = 1ULL << (bit - 1);
		enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(1.0f);
		enemy_subuv[enemy.index].anim_timer = 0.0f;
	}
}

void PlayMode::enemy_dead(Enemy enemy)
{
	enemy_subuv[enemy.index].anim_timer += 0.1f * enemy_subuv[enemy.index].speed;

	uint32_t bit = 1;
	while (true)
	{
		if (enemy_subuv[enemy.index].bitmask >> bit == 0)
			break;
		bit++;
	}

	if (bit == 18) // last frame of dead
		return;
	else
	{
		enemy_subuv[enemy.index].start_index = 9;
		enemy_subuv[enemy.index].range = 8;
		enemy_subuv[enemy.index].speed = 1.0f;

		if (enemy_subuv[enemy.index].anim_timer > 1)
		{
			enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(0.0f);
			bit++;
			enemy_subuv[enemy.index].bitmask = 1ULL << (bit - 1);
			enemy_subuv[enemy.index].subtransforms[bit - 1]->scale = glm::vec3(1.0f);
			enemy_subuv[enemy.index].anim_timer = 0.0f;
		}

		return;
	}
}

void PlayMode::teleport()
{

	if (current_boss->transform->name == final_boss.transform->name)
	{
		if (ready_to_teleport)
		{
			// shrink on old pos
			if (!arrive_new_pos)
			{
				if (teleport_timer > 0)
				{
					detect_boss_status = false;
					current_boss->status = BattleStatus::Weak;
					teleport_timer -= 0.03f;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
				}
				else
				{
					teleport_timer = 0;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
					int ra = rand() % 4;
					auto p = final_teleportPos.begin();
					std::advance(p, ra);
					glm::vec3 nearest_tel = *p;
					current_boss->transform->position = glm::vec3(nearest_tel.x, player->position.y, nearest_tel.z);
					arrive_new_pos = true;
				}
			}
			// expand on new pos
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
		}
	}
	else
	{
		if (ready_to_teleport)
		{
			// shrink on old pos
			if (!arrive_new_pos)
			{
				if (teleport_timer > 0)
				{

					detect_boss_status = false;
					// current_boss->status = BattleStatus::Idle;
					teleport_timer -= 0.03f;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
				}
				else
				{
					teleport_timer = 0;
					current_boss->transform->scale = glm::vec3(teleport_timer, current_boss->transform->scale.y, current_boss->transform->scale.z);
					glm::vec3 nearest_tel = level1_tel->position;
					current_boss->transform->position = glm::vec3(nearest_tel.x, player->position.y, nearest_tel.z);
					arrive_new_pos = true;
				}
			}
			// expand on new pos
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
void PlayMode::change_rand_pos()
{
	int ra = rand() % 9;
	auto p = final_move_pos.begin();
	std::advance(p, ra);
	rand_pos = *p;
}

void PlayMode::play_explode_ani(Boom *boom)
{
	// add animation here

	// after animation finished
	boom->start_explode = false;
	boom->explode->scale = glm::vec3(0);
	boom_count--;
}