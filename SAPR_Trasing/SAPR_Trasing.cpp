#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <queue>

struct Component
{
    //Component params
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

struct Cell
{
    int Id;
    Cell* RightNeighbour = nullptr;
    Cell* LeftNeighbour = nullptr;
    Cell* UpNeighbour = nullptr;
    Cell* DownNeighbour = nullptr;

    PassInfo* PassInfo;

    CellState State;


    Component* component;

    bool IsEqual(Cell anotherCell)
    {
        return Id == anotherCell.Id;
    }
};




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
            case 'C': // Cell contains component
                board[i][j].State = ContainsComponent;
                // Initialize component data
                board[i][j].component = new Component();
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
                std::cout << "@ ";
                break;
            case ContainsComponent:
                std::cout << "C ";
                break;
            case ContainsWire:
                std::cout << "x ";
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
            std::cout << board[i][j].PassInfo->Weight << " ";
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



void MarkShortestPath(Cell** board, Cell* startCell, Cell* endCell) {
    Cell* currentCell = endCell;
    std::vector<Cell*> pathCells;

    // Собираем ячейки пути в вектор
    while (currentCell != startCell) {
        pathCells.push_back(currentCell);
        if (currentCell->DownNeighbour != nullptr && currentCell->DownNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            currentCell = currentCell->DownNeighbour;
        }
        else if (currentCell->UpNeighbour != nullptr && currentCell->UpNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            currentCell = currentCell->UpNeighbour;
        }
        else if (currentCell->RightNeighbour != nullptr && currentCell->RightNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            currentCell = currentCell->RightNeighbour;
        }
        else if (currentCell->LeftNeighbour != nullptr && currentCell->LeftNeighbour->PassInfo->Weight == currentCell->PassInfo->Weight - 1) {
            currentCell = currentCell->LeftNeighbour;
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

void WaveAlgorithm(Cell** board, int rows, int cols, Cell * startCell, Cell * endCell)
{
    // Установка начальной ячейки
    startCell->PassInfo->IsPassed = true;

    // Очередь для обхода ячеек
    std::queue<Cell*> cellsQueue;
    cellsQueue.push(startCell);

    while (!cellsQueue.empty())
    {
        Cell* currentCell = cellsQueue.front();
        cellsQueue.pop();

        // Проверка и обработка соседних ячеек
        if (currentCell->DownNeighbour != nullptr && !currentCell->DownNeighbour->PassInfo->IsPassed)
        {
            currentCell->DownNeighbour->PassInfo->Weight = currentCell->PassInfo->Weight + 1;
            currentCell->DownNeighbour->PassInfo->IsPassed = true;
            cellsQueue.push(currentCell->DownNeighbour);
        }

        if (currentCell->UpNeighbour != nullptr && !currentCell->UpNeighbour->PassInfo->IsPassed)
        {
            currentCell->UpNeighbour->PassInfo->Weight = currentCell->PassInfo->Weight + 1;
            currentCell->UpNeighbour->PassInfo->IsPassed = true;
            cellsQueue.push(currentCell->UpNeighbour);
        }

        if (currentCell->RightNeighbour != nullptr && !currentCell->RightNeighbour->PassInfo->IsPassed)
        {
            currentCell->RightNeighbour->PassInfo->Weight = currentCell->PassInfo->Weight + 1;
            currentCell->RightNeighbour->PassInfo->IsPassed = true;
            cellsQueue.push(currentCell->RightNeighbour);
        }

        if (currentCell->LeftNeighbour != nullptr && !currentCell->LeftNeighbour->PassInfo->IsPassed)
        {
            currentCell->LeftNeighbour->PassInfo->Weight = currentCell->PassInfo->Weight + 1;
            currentCell->LeftNeighbour->PassInfo->IsPassed = true;
            cellsQueue.push(currentCell->LeftNeighbour);
        }
    }

    MarkShortestPath(board, startCell, endCell);

    ClearPassInfo(board, rows, cols);
}

Cell** CreateBoard(int rows, int cols)
{
    Cell** board = new Cell * [rows];
    for (int i = 0; i < rows; ++i)
    {
        board[i] = new Cell[cols];
    }
    return board;
}

void DeleteBoard(Cell** board, int rows)
{
    for (int i = 0; i < rows; ++i)
    {
        delete[] board[i];
    }
    delete[] board;
}

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
    int rows = 16;
    int cols = 16; 

    Cell** board = CreateBoard(rows, cols);

    InitializeBoard(board, rows, cols);

    std::cout << "Board: \n";
    PrintBoard(board, rows, cols);

    // Находим все ячейки, содержащие компоненты
    std::vector<Cell*> componentCells = FindAllElements(board, rows, cols, ContainsComponent);


    WaveAlgorithm(board, rows, cols, componentCells[0], componentCells[2]);

    std::cout << "Board after 1 tracing: \n";
    PrintBoard(board, rows, cols);

    WaveAlgorithm(board, rows, cols, componentCells[1], componentCells[3]);

    std::cout << "Board after 2 tracing: \n";
    PrintBoard(board, rows, cols);

    DeleteBoard(board, rows);

    return 0;
}
