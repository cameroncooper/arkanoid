void doRender();
void doUpdate();
void doArkanoid();
void doBlockSound(int n);
void doPrintNum(int n, int x, int y);

typedef struct {
	int pos;
	int speed;
	int size;
} paddleData;

typedef struct {
	int type;
	int health;
	int points;
	int drop;
} blockData;

typedef struct {
	int x;
	int y;
	int dx;
	int dy;
} ballData;

typedef struct {
	int type;
	int frame;
	int y;
	int dy;
} powerupData;
	

int exp10(int n);
