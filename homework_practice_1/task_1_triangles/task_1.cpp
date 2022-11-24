#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//using namespace glm;

#include <common/shader.hpp>


//Массивы координат и цветов точек отрисовываемых объектов
const GLfloat triangles_vertices[] = {
	-1.0f, -1.0f, -2.0f,
	1.0f, -1.0f, -2.0f,
	-1.0f, 1.0f, -2.0f,
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
};
const GLfloat triangles_colors[] = {
	1.0f, 0.0f, 0.0f, 0.5f,
	1.0f, 0.0f, 0.0f, 0.5f,
	1.0f, 0.0f, 0.0f, 0.5f,
	0.0f, 1.0f, 0.0f, 0.5f,
	0.0f, 1.0f, 0.0f, 0.5f,
	0.0f, 1.0f, 0.0f, 0.5f,
};

int programInitialize(GLFWwindow*& window, GLuint& vertex_buffer, GLuint& color_buffer,
		GLuint& vertex_array, GLuint& program_id) {
	// Инициализируем GLFW
	if (!glfwInit()) {
	    fprintf(stderr, "Ошибка при инициализации GLFW\n");
	    return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x Сглаживание
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Мы хотим использовать OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Мы не хотим старый OpenGL

	// Открываем окно и создаем в нем контекст OpenGL
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(1000, 700, "Задача 1", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Невозможно открыть окно GLFW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Инициализируем GLEW
	glewExperimental = true; // Флаг необходим в Core-режиме OpenGL
	if (glewInit() != GLEW_OK) {
	    fprintf(stderr, "Невозможно инициализировать GLEW\n");
	    getchar();
	    glfwTerminate();
	    return -1;
	}

	// Включим режим отслеживание нажатия клавиш для проверки ниже
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Создание Vertex Array Object'а
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// Загрузка и компиляция шейдеров
	program_id = LoadShaders(
		"TransformTransparencyVertexShader.vertexshader",
		"ColorTransparencyFragmentShader.fragmentshader"
	);

	// Создание буфферов вершин и цветов
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles_vertices), triangles_vertices,
		GL_STATIC_DRAW);
	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles_colors), triangles_colors,
		GL_STATIC_DRAW);

	// Установим цвет фона
	glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

	// Включение механизма прозрачности
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Включение механизма Z-буффера
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return 0;
}

void programFinalize(GLuint& vertex_buffer, GLuint& color_buffer, GLuint& vertex_array,
		GLuint& program_id) {
	// Очистка VBO
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteProgram(program_id);

	// Закрытие окна OpenGL и уничтожение GLFW
	glfwTerminate();
}


int main() {
	GLFWwindow* window;
	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint vertex_array;
	GLuint program_id;
	if (programInitialize(window, vertex_buffer, color_buffer, vertex_array, program_id) != 0)
		return -1;

	// Формирование матрицы преобразования пространства
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
	glm::mat4 matrix_projection =
		glm::perspective(glm::radians(45.0f), 10.0f / 7.0f, 0.1f, 100.0f);
	glm::mat4 matrix_view  = glm::lookAt(
		glm::vec3(2, 0, 2.5),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 1, 0)
	);
	glm::mat4 matrix_model = glm::mat4(1.0f);
	glm::mat4 MVP = matrix_projection * matrix_view * matrix_model;

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Устанонавливаем шейдер текущим
		glUseProgram(program_id);

		// Установка матрицы преобразования пространства
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

	    // Подключение буфферов вершин и цветов
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
		   0,
		   3,					// Размер
		   GL_FLOAT,			// Тип
		   GL_FALSE,			// Указывает, что значения не нормализованы
		   0,					// Шаг
		   (void*)0				// Смещение массива в буфере
		);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glVertexAttribPointer(
			1,
			4,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// Рисование вершин
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

	    // Смена буфферов
	    glfwSwapBuffers(window);
	    glfwPollEvents();

	    // Проверяем нажатие клавиши Escape или закрытие окна
	} while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	programFinalize(vertex_buffer, color_buffer, vertex_array, program_id);

	return 0;
}