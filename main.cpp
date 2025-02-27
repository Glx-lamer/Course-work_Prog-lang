#define _CRT_SECURE_NO_WARNINGS
#include <include/GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_EXPRESSION_LENGTH 100

char userExpression[MAX_EXPRESSION_LENGTH];
float rotateX = 0.0;
float rotateY = 0.0;
float rotateZ = 0.0;
float translateZ = -30.0;
float translateX = 0.0;
float translateY = 0.0;
double diapStart = -4.0;
double diapEnd = 4.0;
double diapStep = 0.01;

struct Point {
    double x;
    double y;
    double z;
};

struct Point* points = NULL;

int IsSimpleOperation(char ch) {
    switch (ch) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '!':
    case '(':
    case ')':
        return 1;
    default:
        return 0;
    }
}

int OperationWeight(int operation) {
    switch (operation) {
    case 1:
        return 0;
    case 2:
    case 3:
        return 1;
    case 4:
    case 5:
        return 2;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 19:
    case 110:
    case 111:
        return 3;
    default:
        return -1;
    }
}

struct QueueNode {
    char* data;
    struct QueueNode* next;
};

struct Queue {
    struct QueueNode* front;
    struct QueueNode* back;
};

struct Queue* createQueue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->front = NULL;
    queue->back = NULL;
    return queue;
}

void enqueue(struct Queue* queue, char* data) {
    struct QueueNode* node = (struct QueueNode*)malloc(sizeof(struct QueueNode));
    node->data = (char*)malloc((strlen(data) + 1) * sizeof(char));
    strcpy(node->data, data);
    node->next = NULL;

    if (queue->back == NULL) {
        queue->front = node;
        queue->back = node;
    }
    else {
        queue->back->next = node;
        queue->back = node;
    }
}

char* dequeue(struct Queue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }
    struct QueueNode* temp = queue->front;
    char* data = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->back = NULL;
    }

    free(temp);
    return data;
}

struct Stack {
    char* data;
    struct Stack* next;
};

void push(struct Stack** top, char* data) {
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->data = (char*)malloc((strlen(data) + 1) * sizeof(char));
    strcpy(stack->data, data);
    stack->next = *top;
    *top = stack;
}

char* pop(struct Stack** top) {
    if (*top == NULL) {
        return NULL;
    }
    struct Stack* temp = *top;
    *top = (*top)->next;
    char* data = temp->data;
    free(temp);
    return data;
}

struct ResStack {
    double data;
    struct ResStack* next;
};

void pushRes(struct ResStack** top, double data) {
    struct ResStack* stack = (struct ResStack*)malloc(sizeof(struct ResStack));
    stack->data = data;
    stack->next = *top;
    *top = stack;
}

double popRes(struct ResStack** top) {
    if (*top == NULL) {
        return 0;
    }
    struct ResStack* temp = *top;
    *top = (*top)->next;
    double data = temp->data;
    free(temp);
    return data;
}

struct DictNode {
    char* key;
    int value;
    struct DictNode* next;
};

struct Dict {
    struct DictNode* head;
};

struct Dict* createDict() {
    struct Dict* dict = (struct Dict*)malloc(sizeof(struct Dict));
    dict->head = NULL;
    return dict;
}

void add(struct Dict* dict, const char* key, int value) {
    struct DictNode* node = (struct DictNode*)malloc(sizeof(struct DictNode));
    node->key = (char*)malloc((strlen(key) + 1) * sizeof(char));
    strcpy(node->key, key);
    node->value = value;
    node->next = dict->head;
    dict->head = node;
}

int get(struct Dict* dict, const char* key) {
    struct DictNode* curr = dict->head;
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return 0;
}

void createDictOperations(struct Dict* dict) {
    add(dict, "(", 1);
    add(dict, ")", -1);
    add(dict, "+", 2);
    add(dict, "-", 3);
    add(dict, "*", 4);
    add(dict, "/", 5);
    add(dict, "^", 6);
    add(dict, "sqrt", 7);
    add(dict, "!", 8);
    add(dict, "sin", 9);
    add(dict, "cos", 10);
    add(dict, "tan", 11);
    add(dict, "arcsin", 19);
    add(dict, "arccos", 110);
    add(dict, "arctan", 111);
}

void Dijkstra(char* operation, struct Dict* dict, struct Queue* queue, struct Stack** stack) {
    if (get(dict, operation) == -1) {
        char* temp;
        do {
            temp = pop(stack);
            if (get(dict, temp) != 1) {
                enqueue(queue, temp);
            }
        } while (get(dict, temp) != 1);
    }
    else if (get(dict, operation) == 1) {
        push(stack, operation);
    }
    else {
        while (*stack != NULL && OperationWeight(get(dict, operation)) <= OperationWeight(get(dict, (*stack)->data))) {
            enqueue(queue, pop(stack));
        }
        push(stack, operation);
    }
}

double OperationResult(int operation, double temp1, double temp2) {
    switch (operation) {
    case 2:
        return temp1 + temp2;
    case 3:
        return temp1 - temp2;
    case 4:
        return temp1 * temp2;
    case 5:
        return temp1 / temp2;
    case 6:
        return pow(temp1, temp2);
    case 7:
        return sqrt(temp2);
    case 8:
        for (int i = (int)temp2 - 1; i > 1; i--) {
            temp2 *= i;
        }
        return temp2;
    case 9:
        return sin(temp2);
    case 10:
        return cos(temp2);
    case 11:
        return tan(temp2);
    case 19:
        return asin(temp2);
    case 110:
        return acos(temp2);
    case 111:
        return atan(temp2);
    default:
        return 0;
    }
}

double calculate_expression(char* expression, double x, double y) {
    struct Dict* DictOperations = createDict();
    createDictOperations(DictOperations);

    struct Queue* queue = createQueue();
    struct Stack* stack = NULL;
    struct ResStack* resstack = NULL;

    char* buffer = NULL;
    int bufferSize = 10;
    int bufferIndex = 0;
    char ch;

    buffer = (char*)malloc(bufferSize * sizeof(char));

    char* expr_ptr = expression;

    while ((ch = *expr_ptr++) != '\0') {
        if (bufferIndex == bufferSize) {
            bufferSize++;
            buffer = (char*)realloc(buffer, bufferSize * sizeof(char));
        }
        if (ch == ' ') {
            continue;
        }
        if (bufferIndex > 0) {
            //11
            if (buffer[bufferIndex - 1] > 47 && buffer[bufferIndex - 1] < 58 && ch > 47 && ch < 58) {
                buffer[bufferIndex++] = ch;
                continue;
            }
            //aa
            else if ((buffer[bufferIndex - 1] < 47 || buffer[bufferIndex - 1] > 58) && (ch < 47 || ch > 58)) {
                if (IsSimpleOperation(ch)) {
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    if (get(DictOperations, buffer) != 0) {
                        Dijkstra(buffer, DictOperations, queue, &stack);
                    }
                    else {
                        enqueue(queue, buffer);
                    }
                    buffer[bufferIndex++] = ch;
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    Dijkstra(buffer, DictOperations, queue, &stack);
                }
                else {
                    buffer[bufferIndex++] = ch;
                    continue;
                }
            }
            //1a
            else if (buffer[bufferIndex - 1] > 47 && buffer[bufferIndex - 1] < 58 && (ch < 47 || ch > 58)) {
                buffer[bufferIndex] = '\0';
                bufferIndex = 0;
                enqueue(queue, buffer);
                buffer[bufferIndex++] = ch;
                if (IsSimpleOperation(ch)) {
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    Dijkstra(buffer, DictOperations, queue, &stack);
                }
                continue;
            }
            //a1
            else if ((buffer[bufferIndex - 1] < 47 || buffer[bufferIndex - 1] > 58) && ch > 47 && ch < 58) {
                buffer[bufferIndex] = '\0';
                bufferIndex = 0;
                Dijkstra(buffer, DictOperations, queue, &stack);
                buffer[bufferIndex++] = ch;
            }
        }
        else {
            buffer[bufferIndex++] = ch;
        }
    }
    if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0';
        bufferIndex = 0;
        if (IsSimpleOperation(buffer[0])) {
            if (get(DictOperations, buffer) != 0) {
                Dijkstra(buffer, DictOperations, queue, &stack);
            }
            else {
                enqueue(queue, buffer);
            }
        }
        else {
            enqueue(queue, buffer);
        }
    }

    while (stack != NULL) {
        enqueue(queue, pop(&stack));
    }

    while (queue->front != NULL) {
        char* temp = dequeue(queue);

        if (temp[0] > 47 && temp[0] < 58) {
            pushRes(&resstack, strtod(temp, NULL));
        }
        else if (get(DictOperations, temp) != 0) {
            if (get(DictOperations, temp) > 6) {
                double temp_ = popRes(&resstack);
                double res = OperationResult(get(DictOperations, temp), 0, temp_);
                pushRes(&resstack, res);
            }
            else {
                double temp2 = popRes(&resstack);
                double temp1 = popRes(&resstack);
                double res = OperationResult(get(DictOperations, temp), temp1, temp2);
                pushRes(&resstack, res);
            }
        }
        else {
            if (temp[0] == 'x') {
                pushRes(&resstack, x);
            }
            if (temp[0] == 'y') {
                pushRes(&resstack, y);
            }
        }
        free(temp);
    }

    free(buffer);
    free(DictOperations);
    free(queue);

    return popRes(&resstack);
}

void computePoints(char* expression) {

    int index = 0;

    for (double x = diapStart; x <= diapEnd; x += diapStep) {
        for (double y = diapStart; y <= diapEnd; y += diapStep) {
            double z = calculate_expression(expression, x, y);
            if (index == 0) {
                points = (struct Point*)malloc(sizeof(struct Point));
            }
            else {
                points = (struct Point*)realloc(points, (index + 1) * sizeof(struct Point));
            }
            struct Point* point = (struct Point*)malloc(sizeof(struct Point));
            point->x = x;
            point->y = y;
            point->z = z;
            points[index++] = *point;
        }
    }
}

void DrawUpdated() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    glTranslatef(translateX, translateY, translateZ);
    glRotatef(rotateX, 1.0, 0.0, 0.0);
    glRotatef(rotateY, 0.0, 1.0, 0.0);
    glRotatef(rotateZ, 0.0, 0.0, 1.0);

    // Ось x
    glBegin(GL_LINES);
    glVertex3f(-100.0, 0.0, 0.0);
    glVertex3f(100.0, 0.0, 0.0);
    glEnd();

    // Ось y
    glBegin(GL_LINES);
    glVertex3f(0.0, -100.0, 0.0);
    glVertex3f(0.0, 100.0, 0.0);
    glEnd();

    // Ось z
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, -100.0);
    glVertex3f(0.0, 0.0, 100.0);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_POINTS);

    for (int i = 1; i < ((diapEnd - diapStart) / diapStep) * ((diapEnd - diapStart) / diapStep); ++i) {
        glVertex3f(points[i].x, points[i].y, points[i].z);
    }

    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    glPopMatrix();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:  // Escape
        exit(0);
        break;
    case 'w':
        translateZ -= 1.0;
        break;
    case 's':
        translateZ += 1.0;
        break;
    case 'a':
        rotateY -= 5.0;
        break;
    case 'd':
        rotateY += 5.0;
        break;
    case 'q':
        translateX -= 1.0;
        break;
    case 'e':
        translateX += 1.0;
        break;
    case 'y':
        rotateX += 5.0;
        break;
    case 'h':
        rotateX -= 5.0;
        break;
    case 'g':
        rotateZ += 5.0;
        break;
    case 'j':
        rotateZ -= 5.0;
        break;
    case 'z':
        translateY -= 0.1;
        break;
    case 'x':
        translateY += 0.1;
        break;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        translateZ += 1.0;
        break;
    case GLUT_KEY_DOWN:
        translateZ -= 1.0;
        break;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(0.7764705882352941f, 0.8745098039215686f, 0.8705882352941177f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glutKeyboardFunc(keyboard);
}

int main(int argc, char** argv) {

    printf("Type expression:\n");
    fgets(userExpression, MAX_EXPRESSION_LENGTH, stdin);
    userExpression[strcspn(userExpression, "\n")] = 0;

    computePoints(userExpression);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1600, 1000);
    glutCreateWindow("3D Graph");

    glewInit();

    init();

    glutDisplayFunc(DrawUpdated);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();

    free(points);
    return 0;
}
