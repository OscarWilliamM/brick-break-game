/* Inclui os headers do OpenGL, GLU, e GLUT */
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cstdio>
#include <iostream> 
#include <SDL2/SDL.h>//biblioteca sdl de som
#include <SDL2/SDL_mixer.h> //biblioteca sdl de som 
#include <cstdlib> // Para usar rand() e srand()
#include <ctime>   // Para usar time()
// Incluímos o arquivo de mapa
#include "map.h"
// Carregar textura png
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define ESC 27   // esc
#define SPACE 32 // espaco
#define ENTER 13 // enter

// Variáveis de áudio
Mix_Chunk* soundBounce = nullptr;
Mix_Chunk* soundGameOver = nullptr;
Mix_Chunk* soundBonus = nullptr;

// Variáveis globais e configurações iniciais
const int MAX_BONUS = 10;
int score = 0, vida = 1;
int brick_color = 0, ball_color = 2, level = 2, paddle_color = 2, text_color = 3, size = 1;
GLfloat twoModel[] = { GL_TRUE };
int game_level[] = { 40, 30, 20 };
float rate = game_level[level];
bool game_over = false;
bool game_paused = false;
int cont_bonus_ativo = 0;
// veriaveis de controle para bonus de diminuir o tamanho da bola
bool ball_small_active = false;

GLfloat brick_color_array[][3] = { {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {1, 1, 1} };
GLfloat paddle_color_array[][3] = { {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 0, 1} };

GLfloat text_color_array[][4] = { {1, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}, {1, 1, 0, 1}, {0.5, 0.5, 0.5, 1} };
GLfloat paddle_size[] = { 2, 4, 6 };

GLuint bricksTextureID; // Carregar a textura dos tijolos
GLuint paddleTextureID; // Carregar a textura da raquete
GLuint ballTextureID; // Carregar a textura da bola
GLuint bonus_textureID; // Carregar a textura do bônus

struct brick_coords {
	GLfloat x;
	GLfloat y;
};

struct Bonus {
	float x, y;
	int tipo; // 0: vida a mais, 1: vida a menos
	bool ativo;
	time_t activation_time; // armazena o tempo de ativação do bonus
};

Bonus bonus[MAX_BONUS];
time_t ball_small_activate_time;

// Array para armazenar as coordenadas dos tijolos
brick_coords brick_array[ROWS][COLUMNS];
GLfloat px, bx = 0, by = -12.94, speed = 0, dirx = 0, diry = 0, start = 0;

// Funcao para carregar a textura da parede usando stb_image
GLuint loadTexture(const char* filename)
{
	int width, height, channels;
	unsigned char* image_data = stbi_load(filename, &width, &height, &channels, 0);

	if (!image_data)
	{
		std::cout << "Erro ao carregar a textura da parede: " << stbi_failure_reason() << std::endl;
		return 0; // Retorne 0 para indicar um erro na carga da textura
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Configura os parametros de filtragem e repeticao da textura (ajuste conforme necessario)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carrega os dados da textura na mem?ria da GPU
	if (channels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	}
	else if (channels == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	}

	// Libera a mem?ria alocada pela fun??o stbi_load (deve ser liberada pelo chamador)
	stbi_image_free(image_data);

	return textureID;
}

void initAudio() {
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
		return;
	}

	// Inicializa o SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cerr << "Erro ao inicializar SDL_mixer: " << Mix_GetError() << std::endl;
		return;
	}

	// Carrega os sons
	soundBounce = Mix_LoadWAV("sounds/bounce.mp3");
	if (!soundBounce) {
		std::cerr << "Erro ao carregar bounce.mp3: " << Mix_GetError() << std::endl;
	}

	soundGameOver = Mix_LoadWAV("sounds/gameover.mp3");
	if (!soundGameOver) {
		std::cerr << "Erro ao carregar gameover.mp3: " << Mix_GetError() << std::endl;
	}
	soundBonus = Mix_LoadWAV("sounds/bonus.mp3");
	if (!soundBonus) {
		std::cerr << "Erro ao carregar bonus.mp3: " << Mix_GetError() << std::endl;
	}
}

// Função para liberar recursos de áudio
void cleanupAudio() {
	Mix_FreeChunk(soundBounce);
	Mix_FreeChunk(soundGameOver);
	Mix_FreeChunk(soundBonus);
	Mix_CloseAudio();
	SDL_Quit();
}

void play_random_music() {
    int random_music = rand() % 5 + 1; // Gera um número aleatório entre 1 e 5
    char music_file[20];
    sprintf(music_file, "sounds/music%d.mp3", random_music);

    Mix_Music *music = Mix_LoadMUS(music_file);
    if (music == NULL) {
        fprintf(stderr, "Falha ao carregar música %s! SDL_mixer Error: %s\n", music_file, Mix_GetError());
        return;
    }

    if (Mix_PlayMusic(music, -1) == -1) {
        fprintf(stderr, "Falha ao tocar música %s! SDL_mixer Error: %s\n", music_file, Mix_GetError());
    }
}

// Função para desenhar o bônus
void draw_bonus() {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bonus_textureID);

	for (int i = 0; i < cont_bonus_ativo; i++) {
		if (bonus[i].ativo) { // Desenha o bônus se estiver ativo
			glPushMatrix();
			glTranslatef(bonus[i].x, bonus[i].y, 0); // Posiciona o bônus 
			glColor3f(1.0, 1.0, 0.0);

			glBegin(GL_QUADS);
			// frente
			glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, 0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, 0.5);
			// tras
			glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, -0.5);
			// esquerda
			glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(-0.5, -0.5, 0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(-0.5, 0.5, 0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, -0.5);
			// direita
			glTexCoord2f(0.0, 0.0); glVertex3f(0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.5, 0.5, -0.5);
			// cima
			glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, 0.5, -0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.5, 0.5, -0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.5, 0.5, 0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, 0.5, 0.5);
			// baixo
			glTexCoord2f(0.0, 0.0); glVertex3f(-0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 0.0); glVertex3f(0.5, -0.5, -0.5);
			glTexCoord2f(1.0, 1.0); glVertex3f(0.5, -0.5, 0.5);
			glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, -0.5, 0.5);
			glEnd();

			glPopMatrix();
		}
	}
	glDisable(GL_TEXTURE_2D);
	glPopAttrib();
}

// atualiza a posicao do bonus
void update_bonus() {
	time_t current_time = time(nullptr);
	for (int i = 0; i < cont_bonus_ativo; i++) {
		if (bonus[i].ativo) {
			bonus[i].y -= 0.1; // Velocidade lenta de queda
			// verifica colisao do bonus com a raquete
			if (bonus[i].y <= -14 && bonus[i].x >= px - paddle_size[size] && bonus[i].x <= px + paddle_size[size]) {
				// Ativa o bônus
				if (bonus[i].tipo == 0) {
					vida++;
				}
				else if (bonus[i].tipo == 1) {
					vida++;
					//vida--;
				}
				else if (bonus[i].tipo == 2) {
					// Ativa o bônus de diminuir o tamanho da bola
					//ball_small_active = true;Coli
					//ball_small_activate_time = current_time;
				}
				bonus[i].ativo = false;
				cont_bonus_ativo--;
				//reproduz o som de colisao da raquete com o bonus
				Mix_PlayChannel(-1, soundBonus, 0);
			}	
			// Remove o bônus se sair da tela
			if (bonus[i].y < -20) {
				bonus[i].ativo = false;
				cont_bonus_ativo--;
			}
		}
	}
	// Verifica se o tempo do bônus de diminuir o tamanho da bola expirou
	if (ball_small_active && difftime(current_time, ball_small_activate_time) >= 10) {
		ball_small_active = false;
	}
}

// 
void active_bonus(float x, float y, int tipo) {
	// Verificar se há algum bônus ativo
	bool bonus_ativo = false;
	for (int i = 0; i < MAX_BONUS; i++) {
		if (bonus[i].ativo) { // Se houver bônus ativo, bonus ativo = true
			bonus_ativo = true;
			break;
		}
	}

	// Se não houver bonus ativo, gerar um novo bonus
	if (!bonus_ativo) {
		for (int i = 0; i < MAX_BONUS; i++) {
			if (!bonus[i].ativo) {
				// Gerar a posição x do bônus dentro dos limites da raquete
				float bonus_x = px - paddle_size[size] + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 * paddle_size[size])));
				bonus[i].x = bonus_x;
				bonus[i].y = y;
				bonus[i].tipo = tipo;
				bonus[i].ativo = true;
				bonus[i].activation_time = time(nullptr);
				cont_bonus_ativo++;
				break;
			}
		}
	}
}

// mostra os textos na tela
void draw_text(const char* text, float x, float y, float scale, float r, float g, float b) {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(r, g, b); // Define a cor do texto
	glTranslatef(x, y, 0);
	glScalef(scale, scale, scale);
	for (const char* p = text; *p; p++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	}
	glEnable(GL_LIGHTING);
	glPopMatrix();
	glPopAttrib();
}
// mostra o score na tela
void draw_score(int value) {
	char score_text[20];
	sprintf(score_text, "Score: %d", value);
	draw_text(score_text, 4.0f, -0.9f, 0.01f, 1.0f, 1.0f, 0.0f);
}
// mostra a vida na tela
void draw_life(int value) {
	char life_text[20];
	sprintf(life_text, "Life: %d", value);
	draw_text(life_text, -10.0f, -0.9f, 0.01f, 1.0f, 1.0f, 0.0f);
}

// Função para desenhar a raquete
void draw_paddle() {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);

	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, paddleTextureID);

	glColor3fv(paddle_color_array[paddle_color]);
	glBegin(GL_QUADS);

	// frente
	glVertex3f(-paddle_size[size] + px, -0.5 - 15, 1.0);
	glVertex3f(paddle_size[size] + px, -0.5 - 15, 1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, 1.0);
	glVertex3f(-paddle_size[size] + px, 1 - 15, 1.0);

	// tras
	glVertex3f(-paddle_size[size] + px, -0.5 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, -0.5 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, -1.0);
	glVertex3f(-paddle_size[size] + px, 1 - 15, -1.0);

	// baixo
	glVertex3f(-paddle_size[size] + px, 1 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, 1.0);
	glVertex3f(-paddle_size[size] + px, 1 - 15, 1.0);

	// cima 
	glVertex3f(-paddle_size[size] + px, 0 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 0 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 0 - 15, 1.0);
	glVertex3f(-paddle_size[size] + px, 0 - 15, 1.0);

	// direita
	glVertex3f(paddle_size[size] + px, -0.5 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, -1.0);
	glVertex3f(paddle_size[size] + px, 1 - 15, 1.0);
	glVertex3f(paddle_size[size] + px, -0.5 - 15, 1.0);

	// esquerda
	glVertex3f(-paddle_size[size] + px, -0.5 - 15, -1.0);
	glVertex3f(-paddle_size[size] + px, 1 - 15, -1.0);
	glVertex3f(-paddle_size[size] + px, 1 - 15, 1.0);
	glVertex3f(-paddle_size[size] + px, -0.5 - 15, 1.0);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopAttrib();
}

// Função para desenhar a bola esférica
void draw_ball() {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ballTextureID);

	GLUquadric* quadric = gluNewQuadric();
	gluQuadricTexture(quadric, GL_TRUE);  // Habilita a textura para a esfera

	glPushMatrix();
	glTranslatef(bx, by, 0);
	if (ball_small_active) {
		gluSphere(quadric, 0.7, 52, 52); 
	}
	else {
		gluSphere(quadric, 1.0, 52, 52);
	}
	glPopMatrix();

	glPushMatrix();
	glTranslatef(bx, by, 0);
	gluSphere(quadric, 1.0, 52, 52);
	glPopMatrix();

	gluDeleteQuadric(quadric);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopAttrib();
}

// Função para desenhar um tijolo
void brick(GLfloat x, GLfloat y, GLfloat z, int tipo) {
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, bricksTextureID);

	if (tipo == 1) {  // tijolo inquebrável
		glColor3fv(brick_color_array[3]);
	}
	else {  // tijolo quebrável
		glColor3fv(brick_color_array[brick_color]);
	}

	glBegin(GL_QUADS);

	// frente
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z + 2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + 3, y, z + 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + 3, y + 1, z + 2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + 1, z + 2.0);

	// tras
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z - 1.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + 3, y, z - 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + 3, y + 1, z - 2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + 1, z - 2.0);

	// baixo
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + 1, z - 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + 3, y + 1, z - 2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + 3, y + 1, z + 2.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + 1, z + 2.0);

	// cima
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z - 2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + 3, y, z - 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + 3, y, z + 2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y, z + 2.0);

	// direita
	glTexCoord2f(1.0, 0.0); glVertex3f(x + 3, y, z - 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + 3, y + 1, z - 2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + 3, y + 1, z + 2.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + 3, y, z + 2.0);

	// esquerda
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z - 2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y + 1, z - 2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + 1, z + 2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y, z + 2.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

// Função para desenhar a grade de tijolos
void draw_bricks() {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT);
    int i, j;
    if (start == 0) {
        for (i = 0; i < ROWS; i++) {
            for (j = 0; j < COLUMNS; j++) {
                brick_array[i][j].x = (GLfloat)(j * 3.2); // Ajuste o espaçamento horizontal
                brick_array[i][j].y = (GLfloat)(i * 2.0); // Espaçamento vertical
            }
        }
    }

    glPushMatrix();
    glTranslatef(-22.5, 5, 0); // Ajuste a posição inicial conforme necessário

    for (i = 0; i < ROWS; i += 1) {
        for (j = 0; j < COLUMNS; j += 1) {
            if (map[i][j] == 0) {
                continue;  // Não desenha nada se for espaço vazio
            }
            brick(brick_array[i][j].x, brick_array[i][j].y, 0, map[i][j]);
        }
    }
    glPopMatrix();
	glPopAttrib();
}

void reset() {
	score = 0;
	vida = 1;
	brick_color = 0;
	ball_color = 2;
	level = 2;
	text_color = 3;
	size = 1;
	rate = game_level[level];
	game_over = false;
	game_paused = false;
	start = 0;
	bx = 0;
	by = -12.94;
	dirx = 0;
	diry = 0;
	px = 0;

	// redesenha o os tijolos no mapa
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMNS; j++) {
			if (map[i][j] == 1) {
				map[i][j] = 1;
			}
			else {
				map[i][j] = 2;
			}
		}
	}
	// Reset bonus
	for (int i = 0; i < MAX_BONUS; i++) {
		bonus[i].ativo = false;
	}
	cont_bonus_ativo = 0;
	glutPostRedisplay();
}

// Função de movimento do mouse
// Função de movimento do mouse
void mousemotion(int x, int y) {
    if (start == 1) {
        px = (x - glutGet(GLUT_WINDOW_WIDTH) / 2) / 20;
        if (px > 15) {
            px = 15.5;
        }
        if (px < -15) {
            px = -15.5;
        }
        if (paddle_size[size] > 4) { // verifica se 
            if (px > 15 - paddle_size[size]) {
                px = 16;
            }
            if (px < -15 + paddle_size[size]) {
                px = -16;
            }
        }
    }
    else
        glutSetCursor(GLUT_CURSOR_INHERIT);
}

// Função para alterar o nível de dificuldade
void change_difficulty(int action) {
	level = action - 1;
}

// Função para tratar o menu
void handle_menu(int action) {}

// Função para alterar o tamanho da raquete
void change_paddle_size(int action) {
	size = action - 1;
}

// Função para adicionar o menu
void addMenu() {

	int submenu1 = glutCreateMenu(change_difficulty);
	glutAddMenuEntry("Easy", 1);
	glutAddMenuEntry("Normal", 2);
	glutAddMenuEntry("Hard", 3);

	int submenu2 = glutCreateMenu(change_paddle_size);
	glutAddMenuEntry("Small", 1);
	glutAddMenuEntry("Medium", 2);
	glutAddMenuEntry("Large", 3);

	glutCreateMenu(handle_menu);
	glutAddSubMenu("Difficulty", submenu1);
	glutAddSubMenu("Paddle Size", submenu2);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Função para lidar com o teclado
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case SPACE:
		if (start == 0 && !game_paused) {
			start = 1;
			dirx = 0.05;
			diry = 0.1;
			speed = 2.0;
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		play_random_music();
		break;
	case 'd': px += 1.5; break;
    case 'a': px -= 1.5; break;
	case ENTER:
		if (game_over) {
			reset();
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		break;
	case ESC:  // Pressionou ESC
		exit(0);
		break;
	default:
		break;
	}
}

void display() {
	GLfloat light_position[] = { 0.0, 0.0, 40.0, 1.0 };
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glTranslatef(0, 0, -45);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -45);
	gluLookAt(0, -0.4, 0.9, 0, 0.15, -0.1, 0, 1, 0); // Câmera

	draw_paddle();
	draw_bricks();
	draw_ball();
	draw_score(score);
	draw_life(vida);
	draw_bonus(); // Desenha os bônus

	if (game_over || vida == 0) { // vida == 0 por conta que pode pegar bonus de -1 vida quando a vida é 1
		draw_text("Game Over!!!", -4.0f, -4.0f, 0.01f, 1.0f, 0.0f, 0.0f);
		draw_text("Tecle 'ENTER' para reiniciar o Jogo", -12.0f, -5.5f, 0.01f, 1.0f, 0.0f, 0.0f);
		bx = 0;
		by = -12.94;
		game_paused = true;
		game_over = true;
	}
	glPopMatrix();
	glutSwapBuffers();
}

void collision_detection() {
	int i, j;
	for (i = 0; i < ROWS; i += 1) {
        for (j = 0; j < COLUMNS; j += 1) {
            if (map[i][j] == 0) {
                continue;
            }
            GLfloat x = brick_array[i][j].x - 22.5; // Ajuste a posição inicial conforme necessário
            GLfloat y = brick_array[i][j].y + 5; // Ajuste a posição inicial conforme necessário
            GLfloat brickWidth = 3.2; // Largura do bloco
            GLfloat brickHeight = 3.0; // Altura do bloco

            if (bx >= x && bx <= x + brickWidth && by >= y && by <= y + brickHeight) {
                if (map[i][j] == 2) {  // bloco quebrável
                    map[i][j] = 0; // Remove o bloco
                    if (rand() % 100 < 10) { // 10% de chance de gerar um bônus ao quebrar um tijolo
                        int bonus_tipo = rand() % 3;
                        active_bonus(brick_array[i][j].x + brickWidth / 2, brick_array[i][j].y, bonus_tipo);
                    }
                    score++;
					// Aumenta a velocidade da bola a cada 3 pontos de score somados
					if (score % 5 == 0) { 
						if (speed == 5.0) {
							speed = 5.0;
						}
						else {
						speed += 0.5;
						}
					}
                }

				if (bx >= x && bx <= x + 0.1) {
					dirx = -fabs(dirx); // Colisão na lateral esquerda
				}
				else if (bx <= x + 3 && bx >= x + 2.9) {
					dirx = fabs(dirx); // Colisão na lateral direita
				}
				else {
					diry = -diry; // Colisão na parte superior ou inferior
				}

				// Reproduz o som de colisão
				Mix_PlayChannel(-1, soundBounce, 0);
				break;
			}
		}
	}

	// Colisão com a raquete
	if (by <= -14 && bx >= px - paddle_size[size] && bx <= px + paddle_size[size]) {
		float sectionWidth = paddle_size[size] / 5;
		float leftSection = px - paddle_size[size] + sectionWidth;
		float centerLeftSection = px - paddle_size[size] + 2 * sectionWidth;
		float centerRightSection = px + paddle_size[size] - 2 * sectionWidth;
		float rightSection = px + paddle_size[size] - sectionWidth;

		if (bx < leftSection) {
			diry = -diry;
			if(dirx > 0.1)
				dirx = -0.1; // Colisão na seção centro-direita
			else
				dirx = -fabs(dirx) - 0.05; 
		}
		else if (bx < centerLeftSection) {
			diry = -diry;
			if(dirx > 0.1)
				dirx = -0.1; // Colisão na seção centro-direita
			else
				dirx = -fabs(dirx) - 0.03; 
		}
		else if (bx > rightSection) {
			diry = -diry;
			if(dirx > 0.1)
				dirx = 0.1; 
			else
				dirx = fabs(dirx) + 0.05; // Colisão na seção direita
		}
		else if (bx > centerRightSection) {
			diry = -diry;
			if(dirx > 0.1)
				dirx = 0.1; 
			else
				dirx = fabs(dirx) + 0.03; 
		}
		else {
			diry = -diry; // Colisão na seção central
			dirx = - 0.01; // A bola vai quase reta para cima
		}
		       // Colisão com a lateral da raquete
        if (bx <= px - paddle_size[size]) {
            dirx = -fabs(dirx); // Colisão na lateral esquerda da raquete
        }
        else if (bx >= px + paddle_size[size]) {
            dirx = fabs(dirx); // Colisão na lateral direita da raquete
        }

		// Reproduz o som de colisão
		Mix_PlayChannel(-1, soundBounce, 0);
}
	// Colisão com as bordas
	if (bx <= -20 || bx >= 20) {
		dirx = -dirx;
		// Reproduz o som de colisão
		Mix_PlayChannel(-1, soundBounce, 0);
	}
	if (by >= 20) {
		diry = -diry;
		// Reproduz o som de colisão
		Mix_PlayChannel(-1, soundBounce, 0);
	}
	if (by <= -14.6) {
		if (vida > 0) {
			vida--;
			start = 0;
			bx = 0;
			by = -12.94;
			dirx = 0;
			diry = 0;
			px = 0;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		if (vida <= 0) { // implementar a lógica do game over
			game_over = true;
			// Reproduz o som de game over
			Mix_PlayChannel(-1, soundGameOver, 0);
		}
	}
}

void update(int value) {
	if (start == 1) {
		bx += dirx * speed;
		by += diry * speed;
		collision_detection(); // chama a função de detecção de colisão
		update_bonus(); // atualiza a posição dos bônus
		draw_bonus(); // desenha os bônus
	}
	glutPostRedisplay(); // atualizacao da tela
	glutTimerFunc(16, update, 0); // chama a funcao de atualizacao a cada 16ms
}

// Funcao de redimensionamento da janela
void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, 0.0);
}

// Funcao principal
int main(int argc, char** argv) {
	srand(static_cast<unsigned int>(time(0)));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Brick Breaker Game 3D");
	

	glClearColor(0.0, 0.0, 0.1, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);

	    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);

    bricksTextureID = loadTexture("texture/brick_texture.png");
    paddleTextureID = loadTexture("texture/ball_texture.png");
    ballTextureID = loadTexture("texture/ball_texture.png");
    bonus_textureID = loadTexture("texture/bonus_texture.png");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mousemotion);

    addMenu();
    glutTimerFunc(rate, update, 0);

    initAudio(); // Inicializa o SDL e SDL_mixer

    glutMainLoop();

    cleanupAudio(); // Limpa os recursos de áudio
    return 0;
}
