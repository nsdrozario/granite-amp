#ifndef NK_SFML_H_
#define NK_SFML_H_

#define GL_GLEXT_PROTOTYPES

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

NK_API nk_context *nk_sfml_init(sf::RenderWindow *window);
NK_API void        nk_sfml_shutdown();

NK_API void        nk_sfml_font_stash_begin(nk_font_atlas **atlas);
NK_API void        nk_sfml_font_stash_end();

NK_API int         nk_sfml_handle_event(const sf::Event &event);
NK_API void        nk_sfml_render(nk_anti_aliasing aa, int max_vertex_buffer, int max_element_buffer);

NK_API void        nk_sfml_device_create();
NK_API void        nk_sfml_device_destroy();

// Extras
NK_API struct nk_image nk_sfml_image(const sf::Texture &texture);
NK_API struct nk_image nk_sfml_image(const sf::Sprite &sprite);

#endif

#ifdef NK_SFML_IMPLEMENTATION

#include <cstring>

#ifdef __APPLE__
	#define NK_SHADER_VERSION "#version 150\n"
#else
	#define NK_SHADER_VERSION "#version 300 es\n"
#endif

struct nk_sfml_device
{
	nk_buffer commands{};
	nk_draw_null_texture null{};

	GLuint vbo{}, vao{}, ebo{};
	GLuint prog{};
	GLuint vert_shader{}, frag_shader{};
	GLint attr_pos{}, attr_uv{}, attr_col{};
	GLint uniform_tex{}, uniform_proj{};
	GLuint font_tex{};
};

struct nk_sfml_vertex
{
	float position[2] = {};
	float uv[2] = {};
	nk_byte col[4] = {};
};

static struct nk_sfml
{
	sf::RenderWindow *window{nullptr};

	nk_sfml_device ogl{};
	nk_context ctx{};
	nk_font_atlas atlas{};
} sfml;

NK_INTERN void nk_sfml_device_upload_atlas(const void *image, int width, int height)
{
	nk_sfml_device *dev = &sfml.ogl;

	glGenTextures(1, &dev->font_tex);
	glBindTexture(GL_TEXTURE_2D, dev->font_tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API nk_context *nk_sfml_init(sf::RenderWindow *window)
{
	sfml.window = window;

	nk_init_default(&sfml.ctx, 0);

	// NOTE(diath): SFML does not allow us to interface with the clipboard
	// sfml.ctx.clip.copy = nk_sfml_clipboard_copy;
	// sfml.ctx.clip.paste = nk_sfml_clipboard_paste;
	sfml.ctx.clip.userdata = nk_handle_ptr(0);

	nk_sfml_device_create();
	return &sfml.ctx;
}

NK_API void nk_sfml_shutdown()
{
	nk_font_atlas_clear(&sfml.atlas);
	nk_free(&sfml.ctx);
	nk_sfml_device_destroy();

	std::memset(&sfml, 0, sizeof(sfml));
}

NK_API void nk_sfml_font_stash_begin(nk_font_atlas **atlas)
{
	nk_font_atlas_init_default(&sfml.atlas);
	nk_font_atlas_begin(&sfml.atlas);

	*atlas = &sfml.atlas;
}

NK_API void nk_sfml_font_stash_end()
{
	int w = 0;
	int h = 0;

	const void *image = nk_font_atlas_bake(&sfml.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	nk_sfml_device_upload_atlas(image, w, h);
	nk_font_atlas_end(&sfml.atlas, nk_handle_id(static_cast<int>(sfml.ogl.font_tex)), &sfml.ogl.null);

	if (sfml.atlas.default_font) {
		nk_style_set_font(&sfml.ctx, &sfml.atlas.default_font->handle);
	}
}

NK_API int nk_sfml_handle_event(const sf::Event &event)
{
	nk_context *ctx = &sfml.ctx;

	if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
		const bool down = (event.type == sf::Event::KeyPressed);
		const bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);
		const sf::Keyboard::Key key = event.key.code;

		if (key == sf::Keyboard::LShift || key == sf::Keyboard::RShift) {
			nk_input_key(ctx, NK_KEY_SHIFT, down);
		} else if (key == sf::Keyboard::Delete) {
			nk_input_key(ctx, NK_KEY_DEL, down);
		} else if (key == sf::Keyboard::Return) {
			nk_input_key(ctx, NK_KEY_ENTER, down);
		} else if (key == sf::Keyboard::Tab) {
			nk_input_key(ctx, NK_KEY_TAB, down);
		} else if (key == sf::Keyboard::BackSpace) {
			nk_input_key(ctx, NK_KEY_BACKSPACE, down);
		} else if (key == sf::Keyboard::Home) {
			nk_input_key(ctx, NK_KEY_TEXT_START, down);
			nk_input_key(ctx, NK_KEY_SCROLL_START, down);
		} else if (key == sf::Keyboard::End) {
			nk_input_key(ctx, NK_KEY_TEXT_END, down);
			nk_input_key(ctx, NK_KEY_SCROLL_END, down);
		} else if (key == sf::Keyboard::PageDown) {
			nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
		} else if (key == sf::Keyboard::PageUp) {
			nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
		} else if (key == sf::Keyboard::Left) {
			if (ctrl) {
				nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
			} else {
				nk_input_key(ctx, NK_KEY_LEFT, down);
			}
		} else if (key == sf::Keyboard::Right) {
			if (ctrl) {
				nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
			} else {
				nk_input_key(ctx, NK_KEY_RIGHT, down);
			}
		} else if (key == sf::Keyboard::Up) {
			nk_input_key(ctx, NK_KEY_UP, down);
		} else if (key == sf::Keyboard::Down) {
			nk_input_key(ctx, NK_KEY_DOWN, down);
		} else if (key == sf::Keyboard::Z) {
			nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && ctrl);
		} else if (key == sf::Keyboard::R) {
			nk_input_key(ctx, NK_KEY_TEXT_REDO, down && ctrl);
		} else if (key == sf::Keyboard::C) {
			nk_input_key(ctx, NK_KEY_COPY, down && ctrl);
		} else if (key == sf::Keyboard::V) {
			nk_input_key(ctx, NK_KEY_PASTE, down && ctrl);
		} else if (key == sf::Keyboard::X) {
			nk_input_key(ctx, NK_KEY_CUT, down && ctrl);
		} else if (key == sf::Keyboard::B) {
			nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && ctrl);
		} else if (key == sf::Keyboard::E) {
			nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && ctrl);
		} else {
			return 0;
		}

		return 1;
	} else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased) {
		const bool down = (event.type == sf::Event::MouseButtonPressed);
		const int x = event.mouseButton.x;
		const int y = event.mouseButton.y;

		if (event.mouseButton.button == sf::Mouse::Left) {
			nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
		}

		if (event.mouseButton.button == sf::Mouse::Middle) {
			nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
		}

		if (event.mouseButton.button == sf::Mouse::Right) {
			nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
		}

		return 1;
	} else if (event.type == sf::Event::MouseMoved) {
		static int prev_x = 0;
		static int prev_y = 0;

		if (ctx->input.mouse.grabbed) {
			const int x = ctx->input.mouse.prev.x;
			const int y = ctx->input.mouse.prev.y;

			nk_input_motion(ctx, x + (event.mouseMove.x - prev_x), y + (event.mouseMove.y - prev_y));

			prev_x = event.mouseMove.x;
			prev_y = event.mouseMove.y;
		} else {
			nk_input_motion(ctx, event.mouseMove.x, event.mouseMove.y);
		}

		return 1;
	} else if (event.type == sf::Event::TextEntered) {
		static nk_uint UnicodeBackSpace = 0x08;
		if (event.text.unicode != UnicodeBackSpace) {
			nk_input_unicode(ctx, event.text.unicode);
			return 1;
		}
	} else if (event.type == sf::Event::MouseWheelScrolled) {
		nk_input_scroll(ctx, event.mouseWheelScroll.delta);
		return 1;
	}

	return 0;
}

NK_API void nk_sfml_render(nk_anti_aliasing aa, int max_vertex_buffer, int max_element_buffer)
{
	static const nk_draw_vertex_layout_element vertex_layout[] = {
		{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(nk_sfml_vertex, position)},
		{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(nk_sfml_vertex, uv)},
		{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(nk_sfml_vertex, col)},
		{NK_VERTEX_LAYOUT_END}
	};

	nk_sfml_device *dev = &sfml.ogl;

	sf::Vector2u size = sfml.window->getSize();

	GLfloat ortho[4][4] = {
		{2.0f, 0.0f, 0.0f, 0.0f},
		{0.0f,-2.0f, 0.0f, 0.0f},
		{0.0f, 0.0f,-1.0f, 0.0f},
		{-1.0f,1.0f, 0.0f, 1.0f},
	};

	ortho[0][0] /= static_cast<GLfloat>(size.x);
	ortho[1][1] /= static_cast<GLfloat>(size.y);

	sf::Vector2u scale{};
	scale.x = size.x / size.x;
	scale.y = size.y / size.y;

	sfml.window->pushGLStates();

	// Global state
	glViewport(0, 0, size.x, size.y);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	// Program
	glUseProgram(dev->prog);
	glUniform1i(dev->uniform_tex, 0);
	glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);

	{
		const nk_draw_command *command{nullptr};
		const nk_draw_index *offset{nullptr};

		void *vertices{nullptr};
		void *elements{nullptr};

		// Allocate data
		glBindVertexArray(dev->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

		glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, nullptr, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, nullptr, GL_STREAM_DRAW);

		// Load vertices and elements into the buffer
		vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		{
			// Fill convert configuration
			nk_convert_config config{};
			NK_MEMSET(&config, 0, sizeof(config));

			config.vertex_layout = vertex_layout;
			config.vertex_size = sizeof(nk_sfml_vertex);
			config.vertex_alignment = NK_ALIGNOF(nk_sfml_vertex);
			config.null = dev->null;
			config.circle_segment_count = 22;
			config.curve_segment_count = 22;
			config.arc_segment_count = 22;
			config.global_alpha = 1.0f;
			config.shape_AA = aa;
			config.line_AA = aa;

			// Setup the buffers
			nk_buffer vbuf{};
			nk_buffer ebuf{};

			nk_buffer_init_fixed(&vbuf, vertices, static_cast<nk_size>(max_vertex_buffer));
			nk_buffer_init_fixed(&ebuf, elements, static_cast<nk_size>(max_element_buffer));

			nk_convert(&sfml.ctx, &dev->commands, &vbuf, &ebuf, &config);
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		// Execute draw commands
		nk_draw_foreach(command, &sfml.ctx, &dev->commands) {
			if (!command->elem_count) {
				continue;
			}

			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(command->texture.id));
			glScissor(
				static_cast<GLint>(command->clip_rect.x * scale.x),
				static_cast<GLint>(size.y - static_cast<GLint>(command->clip_rect.y + command->clip_rect.h) * scale.y),
				static_cast<GLint>(command->clip_rect.w * scale.x),
				static_cast<GLint>(command->clip_rect.h * scale.y)
			);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command->elem_count), GL_UNSIGNED_SHORT, offset);

			offset += command->elem_count;
		}

		nk_clear(&sfml.ctx);
	}

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	sfml.window->popGLStates();
	sfml.window->resetGLStates();
}

NK_API void nk_sfml_device_create()
{
	GLint status;
	static const GLchar *vert_shader =
		NK_SHADER_VERSION
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 TexCoord;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main() {\n"
		"   Frag_UV = TexCoord;\n"
		"   Frag_Color = Color;\n"
		"   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
		"}\n";
	static const GLchar *frag_shader =
		NK_SHADER_VERSION
		"precision mediump float;\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main(){\n"
		"   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
		"}\n";

	nk_sfml_device *dev = &sfml.ogl;
	nk_buffer_init_default(&dev->commands);

	dev->prog = glCreateProgram();
	dev->vert_shader = glCreateShader(GL_VERTEX_SHADER);
	dev->frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(dev->vert_shader, 1, &vert_shader, 0);
	glShaderSource(dev->frag_shader, 1, &frag_shader, 0);

	glCompileShader(dev->vert_shader);
	glCompileShader(dev->frag_shader);

	glGetShaderiv(dev->vert_shader, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);

	glGetShaderiv(dev->frag_shader, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);

	glAttachShader(dev->prog, dev->vert_shader);
	glAttachShader(dev->prog, dev->frag_shader);

	glLinkProgram(dev->prog);
	glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
	assert(status == GL_TRUE);

	dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
	dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
	dev->attr_pos = glGetAttribLocation(dev->prog, "Position");
	dev->attr_uv = glGetAttribLocation(dev->prog, "TexCoord");
	dev->attr_col = glGetAttribLocation(dev->prog, "Color");

	{
		// Buffer setup
		GLsizei vs = sizeof(nk_sfml_vertex);
		size_t vp = offsetof(nk_sfml_vertex, position);
		size_t vt = offsetof(nk_sfml_vertex, uv);
		size_t vc = offsetof(nk_sfml_vertex, col);

		glGenBuffers(1, &dev->vbo);
		glGenBuffers(1, &dev->ebo);
		glGenVertexArrays(1, &dev->vao);

		glBindVertexArray(dev->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

		glEnableVertexAttribArray(static_cast<GLuint>(dev->attr_pos));
		glEnableVertexAttribArray(static_cast<GLuint>(dev->attr_uv));
		glEnableVertexAttribArray(static_cast<GLuint>(dev->attr_col));

		glVertexAttribPointer(static_cast<GLuint>(dev->attr_pos), 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
		glVertexAttribPointer(static_cast<GLuint>(dev->attr_uv), 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
		glVertexAttribPointer(static_cast<GLuint>(dev->attr_col), 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

NK_API void nk_sfml_device_destroy()
{
	nk_sfml_device *dev = &sfml.ogl;

	glDetachShader(dev->prog, dev->vert_shader);
	glDetachShader(dev->prog, dev->frag_shader);

	glDeleteShader(dev->vert_shader);
	glDeleteShader(dev->frag_shader);

	glDeleteProgram(dev->prog);

	glDeleteTextures(1, &dev->font_tex);
	glDeleteBuffers(1, &dev->vbo);
	glDeleteBuffers(1, &dev->ebo);

	nk_buffer_free(&dev->commands);
}

NK_API struct nk_image nk_sfml_image(const sf::Texture &texture)
{
	struct nk_image image{};
	nk_zero(&image, sizeof(image));

	const sf::Vector2u &size = texture.getSize();

	image.handle.id = texture.getNativeHandle();
	image.w = size.x;
	image.h = size.y;

	image.region[0] = 0;
	image.region[1] = 0;
	image.region[2] = size.x;
	image.region[3] = size.y;

	return image;
}

NK_API struct nk_image nk_sfml_image(const sf::Sprite &sprite)
{
	struct nk_image image{};
	nk_zero(&image, sizeof(image));

	const sf::Texture *texture = sprite.getTexture();
	const sf::IntRect &rect = sprite.getTextureRect();

	image.handle.id = texture->getNativeHandle();
	image.w = rect.width;
	image.h = rect.height;

	image.region[0] = rect.left;
	image.region[1] = rect.top;
	image.region[2] = rect.width;
	image.region[3] = rect.height;

	return image;
}

#endif
