#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(WIN32)
#include <windows.h>
void sleepy(int32_t micro_seconds) {
	Sleep(micro_seconds / 1000);
}
#elif defined(__linux__) /* end WIN32 */
#include <unistd.h>
void sleepy(uint64_t micro_seconds) {
	usleep(micro_seconds);
}
#endif /* __linux__ */

#define WIDTH 70
#define HEIGHT 70
#define NUM_PARTICLES 1000
#define FPS 60
#define G 1.0
#define MIN_DIST 0.5
#define SCALE 10.0
#define COLOR_SCALE 0.05

// Unicode blocks for density visualization
const char *density_blocks[] = {"  ", "░░", "▒▒", "▓▓", "██"};
#define NUM_BLOCKS (sizeof(density_blocks) / sizeof(density_blocks[0]))

// Colorful ANSI palette
const char *color_palette[] = {
	"\033[38;5;196m", "\033[38;5;202m", "\033[38;5;208m", "\033[38;5;214m",
	"\033[38;5;220m", "\033[38;5;226m", "\033[38;5;190m", "\033[38;5;154m",
	"\033[38;5;118m", "\033[38;5;82m",  "\033[38;5;46m",  "\033[38;5;47m",
	"\033[38;5;48m",  "\033[38;5;49m",  "\033[38;5;51m",  "\033[38;5;45m",
	"\033[38;5;39m",  "\033[38;5;33m",  "\033[38;5;27m",  "\033[38;5;21m",
	"\033[38;5;57m",  "\033[38;5;93m",  "\033[38;5;129m", "\033[38;5;165m",
	"\033[38;5;201m", "\033[38;5;200m", "\033[38;5;199m", "\033[38;5;198m",
	"\033[38;5;197m", "\033[38;5;196m", "\033[38;5;160m", "\033[38;5;124m",
	"\033[38;5;88m",  "\033[38;5;52m",  "\033[38;5;53m",  "\033[38;5;54m",
	"\033[38;5;55m"
};
#define NUM_COLOR_SHADES (sizeof(color_palette) / sizeof(color_palette[0]))

typedef struct {
	double x, y;
	double vx, vy;
	double ax, ay;
	double mass;
} Particle;

// Map number of particles to block & total speed to color
void density_velocity_to_block_color(
	double n_particles, double speed_sum,
	const char **block, const char **color
) {
	// Block based on particle count
	size_t idx_block = n_particles;
	if (idx_block >= NUM_BLOCKS) idx_block = NUM_BLOCKS - 1;
	if (n_particles > 0.0 && idx_block < 1) idx_block = 1;

	// Color based on speed sum (linear scale, clamp to palette)
	size_t idx_color = (size_t)(speed_sum * COLOR_SCALE);
	if (idx_color >= NUM_COLOR_SHADES) idx_color = NUM_COLOR_SHADES - 1;

	*block = density_blocks[idx_block];
	*color = color_palette[idx_color];
}

void render_frame(double grid_count[HEIGHT][WIDTH], double grid_speed[HEIGHT][WIDTH], char *buffer, size_t buffer_size) {
	printf("\033[H"); // Move cursor to top-left
	char *p = buffer;
	size_t remaining = buffer_size;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			const char *block, *color;
			density_velocity_to_block_color(grid_count[y][x], grid_speed[y][x],
											&block, &color);
			int wrote = snprintf(p, remaining, "%s%s\033[0m", color, block);
			if (wrote < 0 || (size_t)wrote >= remaining) break;
			p += wrote;
			remaining -= wrote;
		}
		int wrote = snprintf(p, remaining, "\n");
		if (wrote < 0 || (size_t)wrote >= remaining) break;
		p += wrote;
		remaining -= wrote;
	}
	printf("%s", buffer);
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	srand((unsigned)time(NULL));

	bool headless = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--headless") == 0) {
			headless = true;
			break;
		}
	}

	Particle *particles = malloc(sizeof(Particle) * NUM_PARTICLES);
	if (!particles) return 1;

	for (int i = 0; i < NUM_PARTICLES; i++) {
		particles[i].x = ((double)rand() / RAND_MAX) * WIDTH * SCALE;
		particles[i].y = ((double)rand() / RAND_MAX) * HEIGHT * SCALE;
		particles[i].vx = (((double)rand() / RAND_MAX) - 0.5) / 2.0;
		particles[i].vy = (((double)rand() / RAND_MAX) - 0.5) / 2.0;
		particles[i].ax = 0;
		particles[i].ay = 0;
		particles[i].mass = 1.0;
	}

	char *buffer = NULL;
	size_t buffer_size = 0;
	if (!headless) {
		buffer_size = HEIGHT * WIDTH * 32 + HEIGHT * 16;
		buffer = malloc(buffer_size);
		if (!buffer) {
			free(particles);
			return 1;
		}
	}

	double elapsed = 0.0;
	size_t ticks = 0;
	clock_t start_time, end_time;

	while (1) {
		ticks++;

		if (headless) start_time = clock();

		double grid_count[HEIGHT][WIDTH] = {0};
		double grid_speed[HEIGHT][WIDTH] = {0};

		// Reset accelerations
		for (int i = 0; i < NUM_PARTICLES; i++) {
			particles[i].ax = 0.0;
			particles[i].ay = 0.0;
		}

		// N-body forces
		for (int i = 0; i < NUM_PARTICLES; i++) {
			for (int j = i + 1; j < NUM_PARTICLES; j++) {
				double dx = (particles[j].x - particles[i].x) / SCALE;
				double dy = (particles[j].y - particles[i].y) / SCALE;
				double dist2 = dx * dx + dy * dy + MIN_DIST;
				double invr = 1.0 / sqrt(dist2);
				double force = G * particles[i].mass * particles[j].mass * invr * invr;
				double fx = force * dx * invr;
				double fy = force * dy * invr;

				particles[i].ax += fx / particles[i].mass;
				particles[i].ay += fy / particles[i].mass;
				particles[j].ax -= fx / particles[j].mass;
				particles[j].ay -= fy / particles[j].mass;
			}
		}

		// Update positions
		for (int i = 0; i < NUM_PARTICLES; i++) {
			particles[i].vx += particles[i].ax;
			particles[i].vy += particles[i].ay;
			particles[i].vx *= 0.99;
			particles[i].vy *= 0.99;
			particles[i].x += particles[i].vx;
			particles[i].y += particles[i].vy;

			int gx = (int)(particles[i].x / SCALE);
			int gy = (int)(particles[i].y / SCALE);

			if (gx >= 0 && gx < WIDTH && gy >= 0 && gy < HEIGHT) {
				grid_count[gy][gx] += 1.0;
				grid_speed[gy][gx] += sqrt(particles[i].vx * particles[i].vx +
										   particles[i].vy * particles[i].vy);
			}
		}

		if (headless) {
			end_time = clock();
			elapsed += (double)(end_time - start_time) / CLOCKS_PER_SEC;
			if (elapsed >= 10.0) break;
		} else {
			render_frame(grid_count, grid_speed, buffer, buffer_size);
			sleepy(1000000 / FPS);
		}
	}

	if (headless) {
		printf("%zu\n", ticks);
	}

	if (!headless) free(buffer);
	free(particles);
}
