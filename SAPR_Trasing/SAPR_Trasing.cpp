#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <queue>
#include <iomanip>

struct Component
{
    std::vector<int> TraceIds; // Номера трасс, которым принадлежит компонент
};

enum CellState
{
    Empty,
    Obstacle,
    ContainsComponent,
    ContainsWire
};

struct PassInfo
{
    bool IsPassed = false;
    int Weight = 0;
};


//Путевая координата
enum Direction
{
    Down,
    Left,
    Up,
    Right

};

struct Cell
{
    int Id;
    Cell* RightNeighbour = nullptr;
    Cell* LeftNeighbour = nullptr;
    Cell* UpNeighbour = nullptr;
    Cell* DownNeighbour = nullptr;

    PassInfo* PassInfo;

    CellState State;
    Direction Direction;


    Component* component;

    bool IsEqual(Cell anotherCell)
    {
        return Id == anotherCell.Id;
    }
};

// Считывание рабочего поля из файла
void InitializeBoard(Cell** board, int rows, int cols)
{
    std::ifstream inputFile("board.txt");

    if (!inputFile.is_open())
    {
        std::cout << "Error opening file." << std::endl;
        return;
    }

    int idGenerator = 10;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            char cellType;
            inputFile >> cellType;

            switch (cellType)
            {
            case 'E': // Empty cell
                board[i][j].State = Empty;
                break;
            case 'O': // Obstacle
                board[i][j].State = Obstacle;
                break;
            case '1': // Cell contains component
                board[i][j].State = ContainsComponent;
                // Initialize component data
                board[i][j].component = new Component();
                board[i][j].component->TraceIds.push_back(1);
                break;
            case '2': // Cell contains component
                board[i][j].State = ContainsComponent;
                // Initialize component data
                board[i][j].component = new Component();
                board[i][j].component->TraceIds.push_back(2);
                break;
            default:
                break;
            }

            board[i][j].Id = idGenerator++;
            board[i][j].PassInfo = new PassInfo();
        }
    }

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            // Установка соседей для внутренних ячеек
            if (i > 0) // Установка верхнего соседа
                board[i][j].UpNeighbour = &board[i - 1][j];
            if (i < rows - 1) // Установка нижнего соседа
                board[i][j].DownNeighbour = &board[i + 1][j];
            if (j > 0) // Установка левого соседа
                board[i][j].LeftNeighbour = &board[i][j - 1];
            if (j < cols - 1) // Установка правого соседа
                board[i][j].RightNeighbour = &board[i][j + 1];
        }
    }

    inputFile.close();
}

// Вывод рабочего поля в консоль
void PrintBoard(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            switch (board[i][j].State)
            {
            case Empty:
                std::cout << ". ";
                break;
            case Obstacle:
                std::cout << "\033[1;31m@ \033[0m"; // Red color for obstaclesR
                break;
            case ContainsComponent:
                std::cout << "\033[1;34mC \033[0m"; // Blue color for components
                break;
            case ContainsWire:
                std::cout << "\033[1;32mx \033[0m"; // Green color for wires
                break;
            default:
                std::cout << "? "; // Unknown state
                break;
            }
        }
        std::cout << std::endl;
    }
    std::cout << '\n';
}
void PrintBoardWeight(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            std::cout << std::setw(3) << board[i][j].PassInfo->Weight << " ";
        }
        std::cout << std::endl;
    }
    std::cout << '\n';
}
void PrintBoardId(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            std::cout << board[i][j].Id << " ";
        }
        std::cout << std::endl;
    }
    std::cout << '\n';
}
void PrintBoardNeighbour(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (board[i][j].DownNeighbour != nullptr)
            {
                std::cout << board[i][j].DownNeighbour->Id << " ";
            }
            else
            {
                std::cout << "00 ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << '\n';
}
void PrintBoardDirection(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            switch (board[i][j].Direction)
            {
            case Down:
                std::cout << "D "; // Стрелка вниз
                break;
            case Up:
                std::cout << "U "; // Стрелка вверх
                break;
            case Left:
                std::cout << "L "; // Стрелка влево
                break;
            case Right:
                std::cout << "R "; // Стрелка вправо
                break;
            default:
                std::cout << "- "; // Нет направления
                break;
            }
        }
        std::cout << std::endl;
    }
    std::cout << '\n';
}


// Очистка информации о прохождении
void ClearPassInfo(Cell** board, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            board[i][j].PassInfo->IsPassed = false;
            board[i][j].PassInfo->Weight = 0;
        }
    }
}


//Прокладка пути
void MarkShortestPath(Cell** board, Cell* startCell, Cell* endCell) {
    Cell* currentCell = endCell;
    std::vector<Cell*> pathCells;

    // Собираем ячейки пути в вектор
    while (currentCell != startCell) {
        pathCells.push_back(currentCell);
        Direction prevDirection = currentCell->Direction;

        // Найти соседа с наименьшим весом и путевыми координатами
        Cell* nextCell = nullptr;
        int minWeight = INT_MAX;
        Direction minDirection = static_cast<Direction>(-1);

        if (currentCell->DownNeighbour != nullptr && currentCell->DownNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            if (currentCell->DownNeighbour->PassInfo->Weight < minWeight || (currentCell->DownNeighbour->PassInfo->Weight == minWeight && currentCell->DownNeighbour->Direction < minDirection)) {
                nextCell = currentCell->DownNeighbour;
                minWeight = nextCell->PassInfo->Weight;
                minDirection = Down;
            }
        }
        if (currentCell->UpNeighbour != nullptr && currentCell->UpNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            if (currentCell->UpNeighbour->PassInfo->Weight < minWeight || (currentCell->UpNeighbour->PassInfo->Weight == minWeight && currentCell->UpNeighbour->Direction < minDirection)) {
                nextCell = currentCell->UpNeighbour;
                minWeight = nextCell->PassInfo->Weight;
                minDirection = Up;
            }
        }
        if (currentCell->RightNeighbour != nullptr && currentCell->RightNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            if (currentCell->RightNeighbour->PassInfo->Weight < minWeight || (currentCell->RightNeighbour->PassInfo->Weight == minWeight && currentCell->RightNeighbour->Direction < minDirection)) {
                nextCell = currentCell->RightNeighbour;
                minWeight = nextCell->PassInfo->Weight;
                minDirection = Right;
            }
        }
        if (currentCell->LeftNeighbour != nullptr && currentCell->LeftNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            if (currentCell->LeftNeighbour->PassInfo->Weight < minWeight || (currentCell->LeftNeighbour->PassInfo->Weight == minWeight && currentCell->LeftNeighbour->Direction < minDirection)) {
                nextCell = currentCell->LeftNeighbour;
                minWeight = nextCell->PassInfo->Weight;
                minDirection = Left;
            }
        }

        if (nextCell != nullptr) {
            currentCell = nextCell;
            currentCell->Direction = minDirection;
        }
        else {
            // Если не найден следующий ход, значит что-то пошло не так
            break;
        }
    }
    pathCells.push_back(startCell);

    // Обновляем состояние доски на основе пути
    for (Cell* cell : pathCells) {
        if (cell != startCell && cell != endCell) {
            cell->State = ContainsWire;
        }
    }
}

int CalculateWeight(Cell* currentCell, Cell* neighbour, Direction currentDirection)
{
    int weight = currentCell->PassInfo->Weight + 1;
    if (currentCell->Direction != neighbour->Direction)
    {
        weight++; // Увеличиваем вес для поворота
    }
    return weight;
}

//Волновой алгоритм, взвешивание ячеек, построение фронта
void WaveAlgorithm(Cell** board, int rows, int cols, Cell * startCell, Cell * endCell)
{
    // Установка начальной ячейки
    startCell->PassInfo->IsPassed = true;
    startCell->Direction = Down;

    // Очередь для обхода ячеек
    std::queue<Cell*> cellsQueue;
    cellsQueue.push(startCell);

    while (!cellsQueue.empty())
    {

        // Выделить веса в отдельную функцию, чтобы учитывать ограничения 
        // Взвешивание по определенным критериям (например минимальное количество пересечений, или минимальное количество изгибов)
        Cell* currentCell = cellsQueue.front();
        cellsQueue.pop();

        // Проверка и обработка соседних ячеек
        if (currentCell->DownNeighbour != nullptr && !currentCell->DownNeighbour->PassInfo->IsPassed)
        {
            int weight = currentCell->PassInfo->Weight + 1;
            if (currentCell->Direction != Down)
            {
                weight++; // Увеличиваем вес для поворота
            }
            currentCell->DownNeighbour->PassInfo->Weight = weight;
            currentCell->DownNeighbour->PassInfo->IsPassed = true;
            currentCell->DownNeighbour->Direction = Down;
            cellsQueue.push(currentCell->DownNeighbour);
        }

        if (currentCell->UpNeighbour != nullptr && !currentCell->UpNeighbour->PassInfo->IsPassed)
        {
            int weight = currentCell->PassInfo->Weight + 1;
            if (currentCell->Direction != Up)
            {
                weight++; // Увеличиваем вес для поворота
            }
            currentCell->UpNeighbour->PassInfo->Weight = weight;
            currentCell->UpNeighbour->PassInfo->IsPassed = true;
            currentCell->UpNeighbour->Direction = Up;
            cellsQueue.push(currentCell->UpNeighbour);
        }

        if (currentCell->RightNeighbour != nullptr && !currentCell->RightNeighbour->PassInfo->IsPassed)
        {
            int weight = currentCell->PassInfo->Weight + 1;
            if (currentCell->Direction != Right)
            {
                weight++; // Увеличиваем вес для поворота
            }
            currentCell->RightNeighbour->PassInfo->Weight = weight;
            currentCell->RightNeighbour->PassInfo->IsPassed = true;
            currentCell->RightNeighbour->Direction = Right;
            cellsQueue.push(currentCell->RightNeighbour);
        }

        if (currentCell->LeftNeighbour != nullptr && !currentCell->LeftNeighbour->PassInfo->IsPassed)
        {
            int weight = currentCell->PassInfo->Weight + 1;
            if (currentCell->Direction != Left)
            {
                weight++; // Увеличиваем вес для поворота
            }
            currentCell->LeftNeighbour->PassInfo->Weight = weight;
            currentCell->LeftNeighbour->PassInfo->IsPassed = true;
            currentCell->LeftNeighbour->Direction = Left;
            cellsQueue.push(currentCell->LeftNeighbour);
        }
    }

    PrintBoardWeight(board, rows, cols);
    PrintBoardDirection(board, rows, cols);

    MarkShortestPath(board, startCell, endCell);

    ClearPassInfo(board, rows, cols);
}

// Выделяем память под рабочее поле
Cell** CreateBoard(int rows, int cols)
{
    Cell** board = new Cell * [rows];
    for (int i = 0; i < rows; ++i)
    {
        board[i] = new Cell[cols];
    }
    return board;
}

// Очищаем память рабочего поля
void DeleteBoard(Cell** board, int rows)
{
    for (int i = 0; i < rows; ++i)
    {
        delete[] board[i];
    }
    delete[] board;
}

// Поиск всех элементов на робочем поле
std::vector<Cell*> FindAllElements(Cell** board, int rows, int cols, CellState state)
{
    std::vector<Cell*> elements;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (board[i][j].State == state)
            {
                elements.push_back(&board[i][j]);
            }
        }
    }
    return elements;
}



int main()
{
    // Персечения в пределах одной цепи запрещены
    // Разные цепи разными цветами
    // Передавать в волнойвой алгоритм массив элементов, принадлежащих одной цепи
    // 2 подхода, цепи по очереди
    int rows = 16;
    int cols = 16;

    Cell** board = CreateBoard(rows, cols);

    InitializeBoard(board, rows, cols);

    std::cout << "Board: \n";
    PrintBoard(board, rows, cols);

    std::vector<Cell*> componentCells = FindAllElements(board, rows, cols, ContainsComponent);

    while (true)
    {

        int component1, component2;
        std::cout << "Enter the numbers of the components you want to connect (from 0 to " << componentCells.size() - 1 << "): ";
        std::cin >> component1 >> component2;

        if (component1 >= 0 && component1 < componentCells.size() && component2 >= 0 && component2 < componentCells.size()) {
            WaveAlgorithm(board, rows, cols, componentCells[component1], componentCells[component2]);

            std::cout << "Board after tracing: \n";
            PrintBoard(board, rows, cols);
        }
        else {
            std::cout << "Invalid component numbers entered. Please enter valid component numbers.\n";
        }
    }

    DeleteBoard(board, rows);

    return 0;
}
