# Brick Break Breakout Game

Este é um jogo de quebra-blocos 3D desenvolvido em C++ utilizando OpenGL e SDL2 para áudio para a disciplina de Computação Gráfica da Universidade Federal do Piauí.

![image](https://github.com/user-attachments/assets/7fd19f3a-e28d-445e-9a1e-75ce586a0a30)

## Funcionalidades
- Movimento da Raquete: Controle a raquete usando o mouse.
- Colisão: Detecção de colisão entre a bola, tijolos e raquete.
- Bônus: Bônus aleatórios que podem aumentar ou diminuir a vida.
- Áudio: Efeitos sonoros para colisões e música de fundo aleatória.
- Dificuldade: Três níveis de dificuldade (Fácil, Normal, Difícil).
- Tamanho da Raquete: Três tamanhos de raquete (Pequeno, Médio,  Grande).

## Controles
- Espaço: Inicia o jogo e toca música aleatória.
- D: Move a raquete para a direita.
- A: Move a raquete para a esquerda.
- Enter: Reinicia o jogo após o game over.
- Esc: Sai do jogo.

Como Executar
1. Certifique-se de ter o SDL2 e SDL2_mixer instalados no seu sistema.
2. Compile o código usando um compilador C++ compatível com OpenGL e SDL2.
3. Execute o binário gerado.

## To run
### Compilar no Linux

```sh
g++ main.cpp -o game -lglut -lGLU -lGL -lSDL2 -lSDL2_mixer -I/usr/include/glm
```

### Compilar no Windows

```sh
g++ main.cpp -o brickbreaker -lglu32 -lopengl32 -lglut32 -lSDL2 -lSDL2_mixer
brickbreaker.exe
```

## Dependências
- OpenGL
- GLUT
- SDL2
- SDL2_mixer
