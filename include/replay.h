typedef struct {
	double wheels[4];
	double prong;
	int last;
} ReplayStep;


void write_replay(ReplayStep *step, char* filename) {
	FILE *f = fopen(filename, "wb");
	for (ReplayStep *i = step; i->last != 1; i++) {
		fwrite(i, sizeof(ReplayStep), 1, f);
	}
	fclose(f);
}

ReplayStep *read_replay(char* filename) {
	FILE *f = fopen(filename, "rb");
	
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	printf("file size: %zu\r\n", size);
	
	ReplayStep* replay = malloc(size);
	ReplayStep currentstep;
	int i = 0;
	while (fread(&currentstep, sizeof(ReplayStep), 1, f)) {
		printf("%lf\r\n", currentstep.prong);
		replay[i].prong = currentstep.prong;
		replay[i].last = 1;
		if (i > 0) {
			replay[i - 1].last = 0;
		}
		for (int ii = 0; ii < 4; ii++) {
			replay[i].wheels[ii] = currentstep.wheels[ii];
		}
		i++;
	}
	fclose(f);
	return replay;
}
