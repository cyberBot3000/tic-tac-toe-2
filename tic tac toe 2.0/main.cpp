#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <random>
#include <stdlib.h> 
#include <Windows.h>
enum class Winner {
	NONE,
	CROSS,
	ZERO,
	TIE
};
enum class Players {
	NONE,
	AI,
	USER,
};
struct Location : public sf::Vector2i{
	Location() 
	{
		row = 0;
		col = 0;
	}
	Location(int row, int col) : row(row), col(col) 
	{
	}
	Location(int fillValue) : row(fillValue), col(fillValue)
	{
	}
	bool operator== (const Location& other)
	{
		return this->row == other.row && this->col == other.col;
	}
	bool inSquare(int leftTop, int rigthBottom)
	{
		return !(row < leftTop || row > rigthBottom || col < leftTop || col > rigthBottom);
		
	}
	int row = 0;
	int col = 0;
};
struct Move {
	Move() {
		boardPos = Location(0, 0);
		cellPos = Location(0, 0);
	}
	Move(Location board, Location cell) {
		boardPos = board;
		cellPos = cell;
	}
	Move(int fillValue) : boardPos(fillValue), cellPos(fillValue)
	{
	}
	bool inSquare(int leftTop, int rightBottom)
	{
		return boardPos.inSquare(leftTop, rightBottom) && cellPos.inSquare(leftTop, rightBottom);
	}
	Location boardPos = Location(0, 0);
	Location cellPos = Location(0, 0);
};
class Cell;
class Line
{
public:
	Line();
	Line(std::vector<Cell*> elements);
	void setChildren(std::vector<Cell*> elements);
	bool contains(Cell* child);
	int getNumInARow();
	Players getProbableWinner();
	void evaluate();
private:
	std::vector<Cell*> children;
	int numInARow;
	int score;
	Players probableWinner;
	std::map<Winner, int> symbolCount;
};
class Cell {
public:
	Cell() 
	{
		this->position = Location(0, 0);
		score = 0;
		winner = Winner::NONE;
	}
	Cell(Location position)
	{
		this->position = position;
		score = 0;
		winner = Winner::NONE;
	}
	virtual Winner getWinner() 
	{
		return winner;
	}
	virtual void setLocation(Location value)
	{
		position = value;
	}
	virtual Location getLocation()
	{
		return position;
	}
	virtual int getWeightToMove()
	{
		return -1;
	}
	virtual void setScore(double score)
	{
		this->score = score;
	}
	virtual int getScore()
	{
		return score;
	}
	virtual bool makeMove(Players currPlayer, Move move = Move())
	{
		if (winner != Winner::NONE) {
			return false;
		}
		switch (currPlayer)
		{
		case Players::AI:
			winner = Winner::ZERO;
			break;
		case Players::USER:
			winner = Winner::CROSS;
			break;
		default:
			winner = Winner::NONE;
			break;
		}
		for (auto& line : lines)
		{
			line.evaluate();
		}
		return true;
	}
	virtual void undoMove(Move position = Move())
	{
		winner = Winner::NONE;
		for (auto& line : lines)
		{
			line.evaluate();
		}
	}
	virtual void addLine(Line line)
	{
		lines.push_back(line);
	}
	virtual std::vector<Line>& getLines()
	{
		return lines;
	}
protected:
	Winner winner;
	Location position;
	int score;
	std::vector<Line> lines;
};
class WithChild : public Cell {
public:
	virtual void getChild(std::vector<std::vector<Cell*> >& cellElements) = 0;
	virtual Cell* childAt(Location position) = 0;
	virtual std::vector<Move> getPossibleMoves() = 0;
private:
};
Line::Line() {
	numInARow = 0;
	score = 0;
	this->probableWinner = Players::NONE;
	symbolCount[Winner::NONE] = 0;
	symbolCount[Winner::CROSS] = 0;
	symbolCount[Winner::ZERO] = 0;
	symbolCount[Winner::TIE] = 0;
}
Line::Line(std::vector<Cell*> elements)
{
	this->children = elements;
	numInARow = 0;
	score = 0;
	symbolCount[Winner::NONE] = 0;
	symbolCount[Winner::CROSS] = 0;
	symbolCount[Winner::ZERO] = 0;
	symbolCount[Winner::TIE] = 0;
}
void Line::setChildren(std::vector<Cell*> elements)
{
	this->children = elements;
}
bool Line::contains(Cell* child) {
	for (auto& item : children)
	{
		if (item == child)
			return true;
	}
	return false;
}
int Line::getNumInARow()
{
	return numInARow;
}
Players Line::getProbableWinner()
{
	return probableWinner;
}
void Line::evaluate()
{
	symbolCount[Winner::NONE] = 0;
	symbolCount[Winner::CROSS] = 0;
	symbolCount[Winner::ZERO] = 0;
	symbolCount[Winner::TIE] = 0;
	for (int i = 0; i < children.size(); i++)
	{
		if (children[i]->getWinner() != Winner::NONE)
		{
			symbolCount[children[i]->getWinner()]++;
		}
		children[i]->setScore(children[i]->getScore() - this->score);
	}
	if (symbolCount[Winner::TIE] > 0 || (symbolCount[Winner::CROSS] > 0 == symbolCount[Winner::ZERO] > 0))
	{
		probableWinner = Players::NONE;
		numInARow = 0;
		score = 0;
	}
	else if (symbolCount[Winner::CROSS] > 0)
	{
		probableWinner = Players::USER;
		numInARow = symbolCount[Winner::CROSS];
		score = pow(10, numInARow + children[0]->getWeightToMove());
	}
	else 
	{
		probableWinner = Players::AI;
		numInARow = symbolCount[Winner::CROSS];
		score = -pow(10, numInARow + children[0]->getWeightToMove());
	}
	for (auto& child : children) {
		child->setScore(child->getScore() + this->score);
	}
}
class LineManager {
public:
	static void makeLines(std::vector<std::vector<Cell*> >& childrenElements)
	{
		std::vector<Cell*> diag1;
		std::vector<Cell*> diag2;
		std::vector<Line> allLines;
		for (int i = 0; i < 3; i++)
		{
			diag1.push_back(childrenElements[i][i]);
			diag2.push_back(childrenElements[i][2 - i]);

			std::vector<Cell*> row;
			std::vector<Cell*> col;
			for (int j = 0; j < 3; j++)
			{
				row.push_back(childrenElements[i][j]);
				col.push_back(childrenElements[j][i]);
			}
			allLines.push_back(Line(row));
			allLines.push_back(Line(col));
		}
		allLines.push_back(Line(diag1));
		allLines.push_back(Line(diag2));

		for (auto& row : childrenElements)
		{
			for (auto& elem : row) 
			{
				for (auto& line : allLines)
				{
					if (line.contains(elem))
					{
						elem->addLine(line);
					}
				}
			}
		}
	}
private:

};
class SmallBoard : public WithChild 
{
public:
	SmallBoard(Location position) {
		this->position = position;
		score = 0;
		winner = Winner::NONE;
		emptyCells = 9;
		cells.resize(3);
		for (int i = 0; i < 3; i++)
		{
			cells[i].resize(3, NULL);
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				cells[i][j] = new Cell(Location(i, j));
			}
		}
		LineManager::makeLines(cells);
	}
	virtual int getWeightToMove() override {
		return 1;
	}
	virtual void getChild(std::vector<std::vector<Cell*> >& cellElements) override
	{
		cellElements = cells;
	}
	virtual Cell* childAt(Location position) override 
	{
		if (position.inSquare(0, 2) == false)
			return nullptr;
		return cells[position.row][position.col];
	}
	virtual bool makeMove(Players currPlayer, Move move = Move()) override
	{
		Location childPos = move.cellPos;
		if (winner != Winner::NONE)
			return false;
		if (childPos.inSquare(0, 2) == false)
			return false;
		Cell* cell = cells[childPos.row][childPos.col];
		this->score -= cell->getScore();
		bool canMove = cell->makeMove(currPlayer);
		this->score += cell->getScore();
		if (!canMove)
			return false;
		emptyCells--;

		if (emptyCells == 0)
		{
			winner = Winner::TIE;
		}
		for (auto& line : cell->getLines())
		{
			if (line.getNumInARow() != 3)
				continue;
			if (winner != Winner::NONE) {
				return false;
			}
			switch (line.getProbableWinner())
			{
			case Players::AI:
				winner = Winner::ZERO;
				break;
			case Players::USER:
				winner = Winner::CROSS;
				break;
			default:
				winner = Winner::NONE;
				break;
			}
			for (auto& line : lines)
			{
				line.evaluate();
			}
			break;
		}
		return true;
	}
	virtual void undoMove(Move move = Move()) override
	{
		Location childPos = move.cellPos;
		if (childPos.inSquare(0, 2) == false)
			return;
		Cell* lastCell = cells[childPos.row][childPos.col];
		score -= lastCell->getScore();
		lastCell->undoMove();
		score += lastCell->getScore();
		emptyCells++;
		this->winner = Winner::NONE;
	}
	virtual std::vector<Move> getPossibleMoves() override 
	{
		std::vector<Move> moves;
		if (winner != Winner::NONE)
		{
			return moves;
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (cells[i][j]->getWinner() != Winner::NONE)
				{
					continue;
				}
				moves.push_back(Move(this->position, Location(i, j)));
			}
		}
		return moves;
	}
private:
	std::vector<std::vector<Cell*> > cells;
	int emptyCells;
};
class BigBoard : public WithChild
{
public:
	BigBoard() 
	{
		emptyBoards = 9;
		boards.resize(3);
		for (int i = 0; i < boards.size(); i++)
		{
			boards[i].resize(3, NULL);
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				boards[i][j] = new SmallBoard(Location(i, j));
			}
		}
		std::vector< std::vector< Cell* > > boardCells;
		boardCells.resize(3);
		for (int i = 0; i < 3; i++)
		{
			boardCells[i].resize(3, NULL);
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				boardCells[i][j] = dynamic_cast<Cell*> (boards[i][j]);
			}
		}
		LineManager::makeLines(boardCells);
	}

	// Унаследовано через WithChild
	virtual void getChild(std::vector<std::vector<Cell*> >& cellElements) override
	{
		cellElements.resize(3);
		for (int i = 0; i < 3; i++)
		{
			cellElements[i].resize(3, NULL);
			for (int j = 0; j < 3; j++)
			{
				cellElements[i][j] = boards[i][j];
			}
		}
	}
	virtual Cell* childAt(Location position)
	{
		return dynamic_cast<Cell*>(boards[position.row][position.col]);
	}
	virtual bool makeMove(Players currPlayer, Move move) override
	{
		if (this->getWinner() != Winner::NONE)
			return false;
		WithChild *currentBoard = boards[move.boardPos.row][move.boardPos.col];
		this->score -= currentBoard->getScore();
		bool canMove = currentBoard->makeMove(currPlayer, move);
		this->score += currentBoard->getScore();
		if (canMove == false)
			return false;
		if (currentBoard->getWinner() != Winner::NONE)
		{
			this->emptyBoards--;
			if (emptyBoards == 0)
				winner = Winner::TIE;
			for (auto& line : currentBoard->getLines())
			{
				if (line.getNumInARow() != 3)
					continue;
				if (winner != Winner::NONE) {
					return true;
				}
				switch (line.getProbableWinner())
				{
				case Players::AI:
					winner = Winner::ZERO;
					break;
				case Players::USER:
					winner = Winner::CROSS;
					break;
				default:
					winner = Winner::NONE;
					break;
				}
				break;
			}
		}
		return true;
	}
	virtual void undoMove(Move move) override
	{
		WithChild* currentBoard = boards[move.boardPos.row][move.boardPos.col];
		if (currentBoard->getWinner() != Winner::NONE)
			emptyBoards++;
		this->score -= currentBoard->getScore();
		currentBoard->undoMove(move);
		this->score += currentBoard->getScore();
		this->winner = Winner::NONE;
	}

	virtual std::vector<Move> getPossibleMoves() override
	{
		std::vector<Move> moves;
		if (this->winner != Winner::NONE)
			return moves;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				std::vector<Move> currentBoardMoves = boards[i][j]->getPossibleMoves();
				moves.insert(moves.end(), currentBoardMoves.begin(), currentBoardMoves.end());
			}
		}
		return moves;
	}
private:
	std::vector< std::vector< WithChild* > > boards;
	int emptyBoards;
};
struct ScoreMove
{
	ScoreMove(int score, Move move) : score(score), move(move)
	{}
	int score = 0;
	Move move = Move(-1);
};
class Game 
{
public:
	Game()
	{
		turn = Players::USER;
		dificulty = 10;
	}
	WithChild* getBigBoiard()
	{
		return dynamic_cast<WithChild*>(&bigBoard);
	}
	WithChild* getCurrentSmallBoard()
	{
		WithChild* smallBoard = NULL;
		if (history.size() == 0)
			return smallBoard;
		Location previousCellLocation = history[history.size() - 1].cellPos;
		smallBoard = dynamic_cast<WithChild*>(bigBoard.childAt(previousCellLocation));
		return smallBoard;
	}
	std::vector<Move> getPosibleMoves()
	{
		WithChild* smallBoard = this->getCurrentSmallBoard();
		if (smallBoard == NULL || smallBoard->getWinner() != Winner::NONE)
		{
			return bigBoard.getPossibleMoves();
		}
		if (bigBoard.getWinner() == Winner::NONE)
		{
			return smallBoard->getPossibleMoves();
		}
		return std::vector<Move>();
	}
	bool makeMove(Move move)
	{
		WithChild* smallBoard = this->getCurrentSmallBoard();
		if (history.size() == 0 || smallBoard->getWinner() != Winner::NONE || smallBoard->getLocation() == move.boardPos)
		{
			bool canMove = bigBoard.makeMove(turn, move);
			if (canMove == false)
				return false;
			if (turn == Players::USER)
				turn = Players::AI;
			else if(turn == Players::AI) turn = Players::USER;
			history.push_back(move);
			return true;
		}
		return false;
	}
	void undoMove()
	{
		if (history.size() == 0)
			return;
		if (turn == Players::USER)
			turn = Players::AI;
		else if (turn == Players::AI) turn = Players::USER;
		Move lastMove = history[history.size() - 1];
		history.pop_back();
		bigBoard.undoMove(lastMove);
	}
	int getScore()
	{
		int score = bigBoard.getScore();
		if (turn == Players::AI)
			score *= -1;
		return score;
	}
	ScoreMove minimax(int depth, Players turn, int maxScore, int minScore)
	{
		
		std::vector<Move> moves = this->getPosibleMoves();
		int score;
		Move bestMove;
		if (moves.size() == 0 || depth == this->dificulty)
		{
			return ScoreMove(this->getScore(), bestMove);
			
		}
		for (auto& move : moves)
		{
			this->makeMove(move);
			score = this->minimax(depth + 1, turn == Players::USER ? Players::AI : Players::USER, maxScore, minScore).score;
			if (turn == Players::AI) 
			{
				if (score > maxScore)
				{
					maxScore = score;
					bestMove = move;
				}
			}
			else
			{
				if (score < minScore)
				{
					minScore = score;
					bestMove = move;
				}
			}
			this->undoMove();
			if (maxScore >= minScore)
			{
				break;
			}
		}
		return ScoreMove(turn == Players::AI ? maxScore : minScore, bestMove);
	}
private:
	BigBoard bigBoard;
	std::vector<Move> history;
	Players turn;
	int dificulty;
	int score;
};
void printBoard(Game& g)
{
	system("cls");
	BigBoard* b = dynamic_cast<BigBoard*>(g.getBigBoiard());
	int threeLineCounter = 0;
	int i = 0;
	int j = 0;
	for (int k = 0; k < 3; k++)
	{
		for (int l = 0; l < 3; l++)
		{
			SmallBoard* currBoard = dynamic_cast<SmallBoard*>(dynamic_cast<WithChild*>(b->childAt(Location(i, j))));
			if (currBoard != nullptr)
				switch (currBoard->childAt(Location(k, l))->getWinner())
				{
				case Winner::CROSS:
					std::cout << "X ";
					break;
				case Winner::ZERO:
					std::cout << "0 ";
					break;
				default:
					std::cout << "- ";
					break;
				}
		}
		std::cout << " ";
		if (j == 2)
		{
			std::cout << std::endl;
			threeLineCounter++;
			if (threeLineCounter % 3 == 0) {
				std::cout << std::endl;
				i++;
				k = -1;
				if (threeLineCounter == 9)
					return;
			}
			j = -1;
			k++;
		}

		j++;
		k--;
	}
}


class CellDisplay : public sf::Drawable
{
public:
	void setPosition();
	void update();
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{

	}
private:

};
class SmallBoardDisplay : public sf::Drawable
{
public:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{

	}
private:

};
class BigBoardDisplay : public sf::Drawable
{
public:
	BigBoardDisplay(BigBoard &board);
	Move getMove(sf::Vector2f cursorCoords);
	void update();
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	BigBoard board;
};


int main() 
{
	std::srand(time(NULL));
	Game g;
	/*g.makeMove(Move(Location(0, 0), Location(2, 2)));
	g.makeMove(Move(Location(2, 2), Location(0, 0)));

	g.makeMove(Move(Location(0, 0), Location(2, 1)));
	g.makeMove(Move(Location(2, 1), Location(0, 0)));

	g.makeMove(Move(Location(0, 0), Location(2, 0)));*/
	while (true)
	{
		printBoard(g);
		std::vector<Move> moves = g.getPosibleMoves();
		if (moves.size() > 0)
		{
			
			Move currMove;
			std::cin >> currMove.boardPos.row >> currMove.boardPos.col >> currMove.cellPos.row >> currMove.cellPos.col;
			if (g.makeMove(currMove))
			{
				printBoard(g);
				Move aiMove = g.minimax(0, Players::AI, INT_MIN, INT_MAX).move;
				g.makeMove(aiMove);
			}
		}
		else break;
	}
	return 0;
}