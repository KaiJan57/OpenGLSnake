#pragma region IncludesAndUsings
//some includes and using namespaces
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#pragma endregion

#pragma region constants
//this is the "Paused-Title"
static const char *titlePaused = "Snake - Paused";
//this is the "Playing-Title"
static const char *titlePlaying = "Snake - Score: ";
//this is the "Default-Title"
static const char *titleDefault = "Snake";
//this is the "Ready-Title"
static const char *titleReady = "Press a button to start";
//this is the default config file
static const string DefaultConfig = 
"#c##Snake Configuration file\n"
"#c##Defines the size of the field (WIDTHxLENGTH)\n"
"Size:\n"
"40x40\n"
"#c##Defines the size of the blocks in pixels\n"
"BlockSize:\n"
"10x10\n"
"#c##Defines the speed (the higher the value, the slower the game)\n"
"Speed:\n"
"100\n"
"#c##Defines the existence of the walls (true = worm dies on walls, false = worm survives the walls)\n"
"Walls:\n"
"false\n"
"#c##Defines the count of foodpoints\n"
"AppleCount:\n"
"2\n"
"#c##The higher this value, the fewer often the value of a foodpoint changes\n"
"AppleSpawn:\n"
"0\n"
"#c##Defines the range of the foodvalues (minimum-maximum)\n"
"AppleValues:\n"
"0-9\n"
"#c##Sets the randomizer (\"time\" for the time)\n"
"Seed:\n"
"time\n"
"#c##Defines the Color of the Borders. Minimum: 0.000 Maximum: 1.000. Dots act as comma. Format: red|green|blue\n"
"Border:\n"
"0.1|0.5|0.4\n"
"#c##Defines the Color of the Background\n"
"Background:\n"
"0.0|0.0|0.0\n"
"#c##Defines the Color of the Worm\n"
"Worm:\n"
"0.0|1.0|0.0\n"
"#c##Defines the Color of the Head\n"
"Head:\n"
"1.0|0.0|0.0";
#pragma endregion

#pragma region DataTypes
//some structs like Point, FoodPoint and Color, so it looks better
struct Point {
	int X = 0;
	int Y = 0;
};

struct FoodPoint : Point {
	int Value = 0;
};

struct Color {
	double r = 0;
	double g = 0;
	double b = 0;
};
#pragma endregion

#pragma region Enums
//enums for the game
//like direction and rendermode
enum Direction {
	Up,
	Down,
	Left,
	Right,
	Paused,
};

enum RenderMode {
	Pause,
	Menu,
	Game,
};
#pragma endregion

#pragma region GLVars
//gl stuff
GLuint VBO;
#pragma endregion

#pragma region GameSettings
//some game settings
int width;
int height;
int blockwidth;
int blockheight;
int applecount;
int applespawn;
int applemin;
int applemax;
long speed;
bool Walls;
Color Border;
Color Background;
Color Worm;
Color Head;
#pragma endregion

#pragma region GameLogicVars
//some variables for the game logics
double bwidth;
double bheight;
long lastupdate;
int score;
int add;
RenderMode renderMode;
vector<Point> WormPoints;
vector<FoodPoint> FoodPoints;
Direction currentDirection;
Direction changeDirection;
Direction pauseDirection;
#pragma endregion

#pragma region FileHandling
//get the file seperator
const char seperator =
#if defined WIN32 || defined _WIN32 || defined __CYGWIN__
'\\';
#include <direct.h>
#define GetCurrentDir _getcwd
#else
'/';
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#pragma endregion

#pragma region DataTypeFuncs
static bool Contains(vector<Point> list, Point point) {
	//check if a list of points contains a single point
	for (vector<Point>::iterator i = list.begin(); i != list.end(); ++i) {
		if (i->X == point.X && i->Y == point.Y) {
			return true;
		}
	}
	return false;
}

static bool Contains(vector<FoodPoint> list, FoodPoint point) {
	//check if a list of foodpoints contains a single foodpoint
	for (vector<FoodPoint>::iterator i = list.begin(); i != list.end(); ++i) {
		if (i->X == point.X && i->Y == point.Y) {
			return true;
		}
	}
	return false;
}
#pragma endregion

#pragma region FileHandling
static string GetStartupPath() {
	//get the current directory or the startup path
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		return NULL;
	}
	return string(cCurrentPath);
}
#pragma endregion

#pragma region RawDrawingMethods
static void Quad(double x, double y, double w, double h, Color colorul, Color colorur, Color colorll, Color colorlr) {
	//draw a raw quad
	glBegin(GL_QUADS);
	glColor3d(colorul.r, colorul.g, colorul.b);
	glVertex2d(x - 1, y + h - 1);
	glColor3d(colorll.r, colorll.g, colorll.b);
	glVertex2d(x - 1, y - 1);
	glColor3d(colorlr.r, colorlr.g, colorlr.b);
	glVertex2d(x + w - 1, y - 1);
	glColor3d(colorur.r, colorur.g, colorur.b);
	glVertex2d(x + w - 1, y + h - 1);
	glEnd();
}

static void Pixel(int x, int y, Color colorul, Color colorur, Color colorll, Color colorlr) {
	//pixel function is just drawing a quad
	Quad(x * bwidth, y * bheight, bwidth, bheight, colorul, colorur, colorll, colorlr);
}

static Color RainBow(double position) {
	//some rainbow function
	if (position == 0) {
		Color color;
		color.r = 1;
		color.g = 1;
		color.b = 1;
		return color;
	}
	double a = (1 - position) / 0.2;
	int X = (int)a;
	int Y = (int)(255 * (a - X));
	int r = 0;
	int g = 0;
	int b = 0;
	switch (X)
	{
	case 0: r = 255; g = Y; b = 0; break;
	case 1: r = 255 - Y; g = 255; b = 0; break;
	case 2: r = 0; g = 255; b = Y; break;
	case 3: r = 0; g = 255 - Y; b = 255; break;
	case 4: r = Y; g = 0; b = 255; break;
	case 5: r = 255; g = 0; b = 255; break;
	}
	Color color;
	color.r = (double)r / (double)225;
	color.g = (double)g / (double)225;
	color.b = (double)b / (double)225;
	return color;
}
#pragma endregion

#pragma region DrawGameElements
static void drawBorder() {
	//just some border
	for (int i = 1; i < width - 1; i++) {
		//draw top and bottom
		Pixel(i, 0, Border, Border, Border, Border);
		Pixel(i, height - 1, Border, Border, Border, Border);
	}
	for (int i = 0; i < height; i++) {
		//draw left and right
		Pixel(0, i, Border, Border, Border, Border);
		Pixel(width - 1, i, Border, Border, Border, Border);
	}
}

static void drawWorm() {
	//draw the worm:
	//get each point
	int j = 0;
	for (vector<Point>::iterator i = WormPoints.begin(); i != WormPoints.end(); ++i) {
		j++;
		if (j == WormPoints.size()) {
			//looks like the head... just draw it. maybe it gets another color...
			Pixel(i->X, i->Y, Head, Head, Head, Head);
		}
		else {
			if (j == WormPoints.size() - 1) {
				//oh and this looks like the second segment of the worm... lets make the color half bodycolor half head color. looks fewer unrealistic
				switch (currentDirection) {
					//and here we have it: the fading mechanism. just see where the worm is going and see how the direction of the fading should be
				case Up: {
					Pixel(i->X, i->Y, Head, Head, Worm, Worm);
					break;
				}
				case Down: {
					Pixel(i->X, i->Y, Worm, Worm, Head, Head);
					break;
				}
				case Left: {
					Pixel(i->X, i->Y, Head, Worm, Head, Worm);
					break;
				}
				case Right: {
					Pixel(i->X, i->Y, Worm, Head, Worm, Head);
					break;
				}
				}
			}
			else {
				//just a normal bodypart... just draw it
				Pixel(i->X, i->Y, Worm, Worm, Worm, Worm);
			}
		}
	}
}

static void drawFood() {
	for (vector<FoodPoint>::iterator i = FoodPoints.begin(); i != FoodPoints.end(); ++i) {
		Color color = RainBow((double)i->Value / (double)10);
		Pixel(i->X, i->Y, color, color, color, color);
	}
}
#pragma endregion

#pragma region GameLogics
static void UpdateFoodPoints() {
	//everyting needed to update the food points
	//remove invisible points
	vector<FoodPoint> ok;
	for (vector<FoodPoint>::iterator i = FoodPoints.begin(); i != FoodPoints.end(); ++i) {
		if (i->X < width || i->Y < height) {
			//this point is ok
			FoodPoint temp;
			temp.X = i->X;
			temp.Y = i->Y;
			temp.Value = i->Value;
			ok.push_back(temp);
		}
	}

	//give good values back
	FoodPoints.clear();
	FoodPoints = ok;

	if (FoodPoints.size() < applecount) {
		//if there are not enough food points, create some
	retry:
		//lets make some randomizing
		FoodPoint temp;
		temp.X = rand() % ((width - 1) - 1) + 1;
		temp.Y = rand() % ((height - 1) - 1) + 1;
		temp.Value = rand() % ((applemax + 1) - applemin) + applemin;
		if (Contains(FoodPoints, temp)) {
			//damn, this position was taken already...
			//nah no problem, lets retry
			goto retry;
		}
		//oh! everything look ok! lets save the point
		FoodPoints.push_back(temp);
	}
	for (vector<FoodPoint>::iterator i = FoodPoints.begin(); i != FoodPoints.end(); ++i) {
		//check if one of the points is in the worm
		//(eating mechanism)
		Point temp;
		temp.X = i->X;
		temp.Y = i->Y;
		if (Contains(WormPoints, temp)) {
			add += i->Value;
			score += i->Value;
			FoodPoints.erase(i);
			glutSetWindowTitle(string(titlePlaying + to_string(score)).c_str());
			break;
		}
	}
	for (vector<FoodPoint>::iterator i = FoodPoints.begin(); i != FoodPoints.end(); ++i) {
		//randomize the values of the apples. BUT ONLY if another randomizer gives us 0
		if (rand() % ((applespawn + 1) - 0) == 0) {
			i->Value = rand() % ((applemax + 1) - applemin) + applemin;
		}
	}
}

static Color stoc(string value) {
	//string to color for parsing this for example: "0.14|0.26|.6"
	int sub1 = value.find('|', 0);
	int sub2 = value.find('|', sub1 + 1);
	double rgb[] = {
		stod(value.substr(0, sub1)),
		stod(value.substr(sub1 + 1, sub2 - sub1 - 1)),
		stod(value.substr(sub2 + 1, value.length() - sub2 - 1)),
	};
	Color temp;
	temp.r = rgb[0];
	temp.g = rgb[1];
	temp.b = rgb[2];
	return temp;
}

static void applyRenderSettings() {
	//Read the config file
	//preset the values, maybe the config file is malformed
	width = 40;
	height = 40;
	blockwidth = 10;
	blockheight = 10;
	speed = 100;
	Walls = false;
	applecount = 2;
	applespawn = 0;
	applemin = 0;
	applemax = 9;
	srand(time(0));

	Border.r = 0.1;
	Border.g = 0.5;
	Border.b = 0.4;

	Background.r = 0;
	Background.g = 0;
	Background.b = 0;

	Worm.r = 0;
	Worm.g = 1;
	Worm.b = 0;

	Head.r = 1;
	Head.g = 0;
	Head.b = 0;

	//get a stream to the file
	ifstream config(GetStartupPath() + seperator + "Config.txt");
	//check if the file exists
	if (config.good()) {
		try
		{
			//good. file exists. read the file into a string
			string temp((istreambuf_iterator<char>(config)), istreambuf_iterator<char>());
			//close the stream so the file is not locked
			config.close();
			//mechanism for saving each line (seperated by "\n") into the vector temp1
			vector<string> temp1;
			string temp2 = "";
			temp += "\n";
			for (int i = 0; i < temp.length(); i++) {
				if (temp[i] == *"\n") {
					temp1.push_back(temp2);
					temp2 = "";
				}
				else {
					temp2 += temp[i];
				}
			}
			//loop through the config-lines
			for (int i = 0; i < temp1.size(); i++) {
				if (temp1[i].find("#c##") != 0) {
					//this line cannot be a comment (begins with #c##)
					string line = temp1[i];
					//first setting: field size
					if (line == "Size:") {
						//get strings seperated by "x"
						string temp_ = temp1[i + 1].substr(0, temp1[i + 1].find("x"));
						string temp__ = temp1[i + 1].substr(temp1[i + 1].find("x") + 1, temp1[i + 1].length());
						//parse them as integer
						int check = stoi(temp_);
						int check1 = stoi(temp__);
						//check if the are bigger than five
						if (check > 5) {
							width = check;
						}
						if (check1 > 5) {
							height = check1;
						}
					}

					//second setting: size per block
					if (line == "BlockSize:") {
						//same procedure as Size of the field
						string temp_ = temp1[i + 1].substr(0, temp1[i + 1].find("x"));
						string temp__ = temp1[i + 1].substr(temp1[i + 1].find("x") + 1, temp1[i + 1].length());
						int check = stoi(temp_);
						int check1 = stoi(temp__);
						if (check > 0) {
							blockwidth = check;
						}
						if (check1 > 0) {
							blockheight = check1;
						}
					}

					//third setting: gamespeed
					if (line == "Speed:") {
						//just parse and check
						int check = stoi(temp1[i + 1]);
						if (check > -1) {
							speed = check;
						}
					}


					//fourth setting: walls
					if (line == "Walls:") {
						//just check if they are true or something else
						if (temp1[i + 1] == "true") {
							Walls = true;
						}
					}

					//fifth setting: count of apples
					if (line == "AppleCount:") {
						//just parse and check if they have a valid value
						int check = stoi(temp1[i + 1]);
						if (check > 0) {
							applecount = check;
						}
					}

					//sixth setting: probability of changing the value of the apples
					if (line == "AppleSpawn:") {
						int check = stoi(temp1[i + 1]);
						if (check > -1) {
							applespawn = check;
						}
					}

					//seventh setting: min and max value for an apple
					if (line == "AppleValues:") {
						//values are sperated by "-"
						string temp_ = temp1[i + 1].substr(0, temp1[i + 1].find("-"));
						string temp__ = temp1[i + 1].substr(temp1[i + 1].find("-") + 1, temp1[i + 1].length());
						int check = stoi(temp_);
						int check1 = stoi(temp__);
						if (check > -1 && check1 > 0 && check1 >= check) {
							applemin = check;
							applemax = check1;
						}
					}

					//eight setting: seed for the randomizer
					if (line == "Seed:") {
						if (temp1[i + 1] != "time") {
							srand(stoi(temp1[i + 1]));
						}
					}

					//nineth - 12th setting: Colors
					if (line == "Border:") {
						Border = stoc(temp1[i + 1]);
					}

					if (line == "Background:") {
						Background = stoc(temp1[i + 1]);
					}

					if (line == "Worm:") {
						Worm = stoc(temp1[i + 1]);
					}

					if (line == "Head:") {
						Head = stoc(temp1[i + 1]);
					}
				}
			}
		}
		catch (const std::exception&)
		{
			//oups looks like there was an error... who cares the default settings were defined up there
		}
	}
	else {
		//oh looks like you are new to the game
		//close the stream
		config.close();
		//create an outputfilestream
		ofstream writeFile(GetStartupPath() + seperator + "Config.txt");
		//and write the example configuration to the file
		writeFile << DefaultConfig;
		//close the stream
		writeFile.close();
	}

	//get opengl width and height from our information
	bwidth = (double)2 / (double)width;
	bheight = (double)2 / (double)height;
}

static void initGame() {
	//initialize the game
	//used for initializing or resetting the game
	//read Config.txt file
	applyRenderSettings();

	//set the game vars
	score = 0;
	add = 0;
	pauseDirection = Right;
	changeDirection = Right;
	currentDirection = Right;
	WormPoints.clear();
	renderMode = Menu;
	//set the initial poits of the WormPoints list
	int j = 0;
	for (int i = 4; i > 0; i--) {
		WormPoints.push_back(Point());
		WormPoints[j].X = width / 2 - i;
		WormPoints[j].Y = height / 2;
		j++;
	}
}

static void Lost() {
	//tell the rendering section to render the "Pause-Screen"
	renderMode = Pause;
	//and set the last update to now, so the "Pause-Screen" is shown for a second
	lastupdate = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

static void moveWorm(Direction direction) {
	//game logics to move the worm
	if (direction == Paused) {
		//if the direction the worm should moved to is paused:
		return;
		//... return!
	}
	//this is for making the worm longer. "add" stores the value, the worm should get longer
	if (add > 0) {
		//if add is not 0, decrease the value (everything per frame!!)
		//notice in this case the worms tail does not get deleted -> worm gets longer
		add--;
	}
	else {
		//otherwise the worm can move normally (delete the last point)
		WormPoints.erase(WormPoints.begin());
	}
	//set the worms head
	Point temp;
	if (direction == Up) {
		//if we are going up or down, the new heads X coordinate equals the old heads X coordinate
		temp.X = WormPoints.back().X;
		//but the Y coordinate is increased
		temp.Y = WormPoints.back().Y + 1;
	}
	if (direction == Down) {
		temp.X = WormPoints.back().X;
		//but the Y coordinate is decreased
		temp.Y = WormPoints.back().Y - 1;
	}
	if (direction == Left) {
		//if we are going left or right, the new heads Y coordinate equals the old heads Y coordinate
		//but the X coordinate is decreased
		temp.X = WormPoints.back().X - 1;
		temp.Y = WormPoints.back().Y;
	}
	if (direction == Right) {
		//but the X coordinate is increased
		temp.X = WormPoints.back().X + 1;
		temp.Y = WormPoints.back().Y;
	}
	//check for collisions
	if (temp.Y > height - 2) {
		//if the worm is too high
		if (Walls) {
			//lose or
			Lost();
		}
		else {
			//come from the bottom again
			temp.X = WormPoints.back().X;
			temp.Y = 1;
		}
	}
	//now self explanatory
	if (temp.Y < 1) {
		if (Walls) {
			Lost();
		}
		else {
			temp.X = WormPoints.back().X;
			temp.Y = height - 2;
		}
	}
	if (temp.X < 1) {
		if (Walls) {
			Lost();
		}
		else {
			temp.X = width - 2;
			temp.Y = WormPoints.back().Y;
		}
	}
	if (temp.X > width - 2) {
		if (Walls) {
			Lost();
		}
		else {
			temp.X = 1;
			temp.Y = WormPoints.back().Y;
		}
	}
	//check if worm bit itself
	if (Contains(WormPoints, temp)) {
		Lost();
	}
	//now the currentDirection is also the changeDirection
	currentDirection = changeDirection;
	//add the new head into the WormPoints list
	WormPoints.push_back(temp);
}
#pragma endregion

#pragma region Renderers
static void RenderGame() {
	//render the game:
	//clear the screen (title is setted somewhere else)
	glClear(GL_COLOR_BUFFER_BIT);
	//do some gl stuff
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//draw the border
	drawBorder();
	//draw the food
	drawFood();
	//draw the worm
	drawWorm();
	//do some more gl stuff
	glDisableVertexAttribArray(0);
	//bring it to screen
	glutSwapBuffers();
}

static void RenderMenu() {
	//render the menu:
	//set window title
	glutSetWindowTitle(titleReady);
	//clear screen
	glClear(GL_COLOR_BUFFER_BIT);
	//do some gl stuff
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//draw the border
	drawBorder();
	//do some more gl stuff
	glDisableVertexAttribArray(0);
	//bring it to screen
	glutSwapBuffers();
}

static void Render() {
	//clear the screen with the user-defined color
	glClearColor(Background.r, Background.g, Background.b, 0);
	switch (renderMode) {
		//check the rendermode
	case Pause: {
		//if it is "Pause" it means pause OR stop the screen if the player lost, so the player knows why the game was lost
		RenderGame();
		long now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		if (lastupdate + 1000 <= now) {
			initGame();
		}
		break;
	}
	case Menu: {
		//self explanatory [who doesn't know: it's for rendering the menu (who would have thought that!)]
		RenderMenu();
		break;
	}
	case Game: {
		//here the game plays all through the logics and renders everything
		RenderGame();
		long now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		//speed is the user-defined game speed
		if (lastupdate + speed <= now) {
			lastupdate = now;
			if (changeDirection != Paused) {
				currentDirection = changeDirection;
				moveWorm(currentDirection);
				UpdateFoodPoints();
			}
		}
		break;
	}
	}
}
#pragma endregion

#pragma region Callbacks
static void Button(int key, int x, int y) {
	if (changeDirection == Paused) {
		//if button is pressed and game is paused, resume the game
		changeDirection = pauseDirection;
		glutSetWindowTitle(string(titlePlaying + to_string(score)).c_str());
		return;
	}

	//self explanatory [who doesn't know: it's for the controls ;-)]
	if (key == GLUT_KEY_UP && currentDirection != Down) {
		changeDirection = Up;
	}
	if (key == GLUT_KEY_DOWN && currentDirection != Up) {
		changeDirection = Down;
	}
	if (key == GLUT_KEY_LEFT && currentDirection != Right) {
		changeDirection = Left;
	}
	if (key == GLUT_KEY_RIGHT && currentDirection != Left) {
		changeDirection = Right;
	}
	if (renderMode == Menu) {
		//if user is in menu and pressed any button, start the game
		glutSetWindowTitle(string(titlePlaying + to_string(score)).c_str());
		renderMode = Game;
		return;
	}
}

static void KeyBoard(unsigned char c, int x, int y) {
	if (c == ' ' && renderMode == Game && changeDirection != Paused) {
		//if pressed space, pause the game
		pauseDirection = changeDirection;
		changeDirection = Paused;
		glutSetWindowTitle(titlePaused);
		//maybe the configuration is changed? just check
		applyRenderSettings();

		//resize the window
		glutReshapeWindow(width * blockwidth, height * blockheight);
		return;
	}
	if (c == 'x') {
		//if pressed X, exit the program
		exit(0);
	}
	if (renderMode == Menu) {
		//if user is in menu and pressed any button, start the game
		glutSetWindowTitle(string(titlePlaying + to_string(score)).c_str());
		renderMode = Game;
		return;
	}
	if (changeDirection == Paused) {
		//if game is paused and user pressed any button, resume the game
		changeDirection = pauseDirection;
		glutSetWindowTitle(string(titlePlaying + to_string(score)).c_str());
		return;
	}

	//self explanatory [who doesn't know: it's for the controls ;-)]
	if (c == 'w' && currentDirection != Down) {
		changeDirection = Up;
	}
	if (c == 's' && currentDirection != Up) {
		changeDirection = Down;
	}
	if (c == 'a' && currentDirection != Right) {
		changeDirection = Left;
	}
	if (c == 'd' && currentDirection != Left) {
		changeDirection = Right;
	}
}


static void InitializeGlutCallbacks()
{
	//initialize all the callbacks
	glutDisplayFunc(Render);
	glutIdleFunc(Render);
	glutSpecialFunc(Button);
	glutKeyboardFunc(KeyBoard);
}

#pragma endregion

int main(int argc, char** argv)
{
	//hide Console if on Windows
#if defined WIN32 || defined _WIN32 || defined __CYGWIN__
	FreeConsole();
#endif
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	initGame();
	//set the window
	glutInitWindowSize(width * blockwidth, height * blockheight);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow(titleDefault);

	InitializeGlutCallbacks();

	//must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	glutMainLoop();

	return 0;
}