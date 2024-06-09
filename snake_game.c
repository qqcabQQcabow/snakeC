#include <ncurses.h>
#include <stdio.h> // стандартный ввод-вывод
#include <stdlib.h> // для system и malloc
#include <termios.h> // для getch()
#include <pthread.h> // потоки
#include <unistd.h> // usleep()
#include <stdbool.h> // для булевых переменных
#include <time.h>

#define X_MAX 50 // ширина поля
#define Y_MAX 37 // высота поля
#define X_START 30 // начальные координаты головы
#define Y_START 6
#define SPEED_MS 100000 // скорость обновления карты
#define BODY '*'
#define TAIL '.'
#define HEAD 'O'

typedef struct sn_node{ // кусочек змеи
    int x;
    int y;
    struct sn_node* next;
    struct sn_node* prev;
}snake_node;

typedef struct eat{ // яблоко
    int x;
    int y;
}apple_coord;

snake_node* CreateNode(int x, int y){ // создаёт узел змеи с координатами x y
    snake_node* new = (snake_node*)malloc(sizeof(snake_node));
    new->x = x;
    new->y = y;
    new->prev = new->next = NULL;
    return new;
}

char move_head; // Символ, который определяет направление движения головы
char map[Y_MAX][X_MAX]; // карта
int x=X_START,y=Y_START; // стартовые позиции головы
int last_move; // стороны света
apple_coord apple; // яблоко(его координаты)
int eateen_apple=0; // счётчик съеденых яблок
int speed = SPEED_MS;

snake_node* GetTail(snake_node* snake){ // получить координаты хвоста
    if((snake->prev) == NULL) return snake;
    else return GetTail(snake->prev);
}

snake_node* GetHead(snake_node* snake){ // получить координаты головы
    if((snake->next) == NULL) return snake;
    else return GetHead(snake->next);
}

void PushEnd(snake_node* snake, int x, int y){ // добавить узел с координатами x y в конец списка - начало змеи
    snake_node* prev_new_head = GetHead(snake);
    snake_node* future_head = CreateNode(x, y);
    future_head->prev=prev_new_head;
    future_head->next = NULL;
    prev_new_head->next = future_head;

}
snake_node* RemoveTail(snake_node* snake){ // удаление хвоста
    snake_node* new_tail = snake->next;
    new_tail->prev = NULL;
    free(snake);
    return new_tail;
}

int FillMap(snake_node* snake){ // заполнение змейкой подготовленной карты
    if(snake->next == NULL){ // если нашли голову, то меняем её скин и проверяем, не уперлась ли она в тело
        if((map[snake->y][snake->x] == BODY)||(map[snake->y][snake->x] == TAIL)) return 1;
        map[snake->y][snake->x] = HEAD;
    }
    else if(snake->prev == NULL){ // скин хвоста
        map[snake->y][snake->x] = TAIL;
    }
    else {
        map[snake->y][snake->x] = BODY;
    } // скин тела
    if(snake->next == NULL) return 0;
    else FillMap(snake->next);
}

int CountNode(snake_node* snake, int count){ // подсчёт узлов змеи
    if(snake->next == NULL) return count;
    else return CountNode(snake->next, count+1);
}

snake_node* snake; // главная героиня - змея, хранит адрес зада

int MoveSnake(snake_node* snaked){ // самостоятельное передвижение змейки зависимо от направления
    snake_node* head = GetHead(snaked); // голова змеи
    snake_node* tail = GetTail(snaked); // зад змеи
    int apple_in_snake=1;
    // изменение направления исходя из нажатой клавиши
    if(last_move == 1){
        if(head->y == 1){ // проверка на то, не уперлась ли змея в стену
            return 1;
        }
        else{
            PushEnd(snaked, head->x, head->y-1); // создаём новую голову с соответствующими координатами
        }
    }
    else if(last_move == 2){ // аналогично с 1
        if(head->x == 1){
            return 1;
        }
        else{
            
            PushEnd(snaked, head->x-1, head->y);
        }
    }
    else if(last_move == 3){ // аналогично с 1
        if(head->y == Y_MAX-2){
            return 1;
        }
        else{
            PushEnd(snaked, head->x, head->y+1);
        }
    }
    else if(last_move == 4){ // аналогично с 1
        if(head->x == X_MAX-2){
            return 1;
        }
        else{
            PushEnd(snaked, head->x+1, head->y);
        }
    }
    else{
        return 0;
    }
    if(CountNode(snaked, 0) == (Y_MAX-1)*(X_MAX-1)){
        return 1;
    }
    for(int i = 0; i < Y_MAX; i++){ // заполнение карты пробелами, яблоком, стенками
        for(int j = 0; j < X_MAX; j++){
            if((apple.x == j) && (apple.y == i)){ // яблоко
                map[i][j] = 'A';
                continue;
            }
            map[i][j]=' '; // пустое место на карте
            if((i == Y_MAX-1) || ( i == 0)) map[i][j]='-'; // верхняя стенка
            else if(( j == X_MAX-1) || (j == 0)) map[i][j] ='|'; // боковая стенка
        }
    }
    if((head->y != apple.y) || (head->x != apple.x)) snake = RemoveTail(snake); // если не попали на яблоко, то хвост удаляем -> змея не растёт
    
    if ((head->y == apple.y) && (head->x == apple.x)){ // обновление координат яблока
        eateen_apple++;
        apple.x = 1 + rand() % (X_MAX-2 - 1);
        apple.y = 1 + rand() % (Y_MAX-2 - 1);
        do{
            if(map[apple.y][apple.x] != ' '){
                apple.x = 1 + rand() % (X_MAX-2 - 1);
                apple.y = 1 + rand() % (Y_MAX-2 - 1);
            }
            else{
                apple_in_snake = 0;
            }
        }while(apple_in_snake);
    }
    if(FillMap(snake)){ // заполнение карты змеёй, если змейка замкнулась, то срабатывает
        return 1;
    }
    return 0;
}



void print_snake(snake_node* snake){ // вывод всех узлов змеи
    printw("pre=%p x=%d y=%d adrr=%p next=%p\n",snake->prev, snake->x, snake->y, snake, snake->next);
    if(snake->next == NULL) return;
    else print_snake(snake->next);
}

void free_snake(snake_node* snake){ // освобождение памяти, которую занмает змея
    if(snake->next != NULL) free_snake(snake->next);
    else{
        free(snake);
        return;
    }
}

bool theard2_stop; // флаги потоков
bool theard1_stop;
pthread_mutex_t mapm = PTHREAD_MUTEX_INITIALIZER; 
void* thread_function2(void* arg) {
    int i=0; // счётчик итераций
    while(1) {
        if(theard1_stop){
            pthread_mutex_unlock(&mapm);
            return NULL;
        }
        pthread_mutex_lock(&mapm);
        erase();
        for(int i = 0; i < Y_MAX; i++){
            for(int j = 0; j < X_MAX; j++){
                printw("%c",map[i][j]);
            }
            printw("\n");
        }
        i++;
        printw("score: %d     speed %d      length: %d\n", eateen_apple, speed, CountNode(snake, 1));
        refresh();
        pthread_mutex_unlock(&mapm);
        usleep(speed);
        if(MoveSnake(snake)){ // если змея куда-то уперлась, то конец игры
            theard2_stop = true;
            pthread_mutex_unlock(&mapm);
            return NULL;
        }  
    }
    return NULL;
}

void* thread_function1(void* arg) {

    while(1){
        if(theard2_stop) {
            printw("stoped!\n");
            pthread_mutex_unlock(&mapm);
            return NULL;
        }
        usleep(speed);
        move_head = getch();
        pthread_mutex_lock(&mapm);
        if(move_head == 'Q'){
            pthread_mutex_unlock(&mapm);
            theard1_stop = true;
            return NULL;
        }
        if((move_head == 'w') && (last_move != 3)){
            //y--;
            if(last_move == 1){
                speed = SPEED_MS/2.35; // this speed only for square letter
            }
            else speed = SPEED_MS;
            last_move=1;
        }
        else if((move_head == 's') && (last_move != 1)){
            //y++;
            if(last_move == 3){
                speed = SPEED_MS/2.35;
            }
            else speed = SPEED_MS;
            last_move=3;
        }
        else if((move_head == 'a') && (last_move != 4)){
            //x--;
            if(last_move == 2){
                speed = SPEED_MS/2.35;
            }
            else speed = SPEED_MS;
            last_move=2;
        }
        else if((move_head == 'd') && (last_move != 2)){
            //x++;
            if(last_move == 4){
                speed = SPEED_MS/2.35;
            }
            else speed = SPEED_MS;
            last_move=4;
        }
        pthread_mutex_unlock(&mapm);
    }
     return NULL;
}

int main() {
    srand(time(NULL));
    initscr();
    noecho();
    apple.x = 2; // первичные координаты яблока
    apple.y = 2;
    
    last_move = 4; // первичное направление движения
    
    theard2_stop = false;
    theard1_stop = false;
    
    snake = CreateNode(10,10); // создание змеи
    for(int i = 15; i <=25 ; i++){
        PushEnd(snake, 10, i);
    }
    
    pthread_t keyboarding_handler, main_game_cycle; // два потока: для обработки клавиш и для главного движения змеи
    
    // Создайте и запустите поток главного цикла
    if (pthread_create(&main_game_cycle, NULL, thread_function2, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    // Создайте и запустите поток обработки клавиш
    if (pthread_create(&keyboarding_handler, NULL, thread_function1, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    // Дождитесь завершения обоих потоков
    pthread_join(keyboarding_handler, NULL);
    pthread_join(main_game_cycle, NULL);
    endwin();
    if(CountNode(snake, 1) == (Y_MAX-1)*(X_MAX-1)){
        printf("You win!!!\n");
    }
    else{
        printf("You lose, score is %d\n", eateen_apple);
    }
    free_snake(snake);
    return 0;
}
