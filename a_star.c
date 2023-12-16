#include <stdio.h>
#include <math.h>

#include "DSA.h"

#define GRID_X 10
#define GRID_Y 10

#define WALKABLE(x, y) (grid[y][x])
#define F_SCORE(GridNode) ((GridNode).gScore + (GridNode).hScore)
#define POSITION_EQUAL(p1, p2) ((p1).x == (p2).x && (p1).y == (p2).y)

typedef struct Position
{
    unsigned int x, y;
} Position;

typedef struct GridNode
{
    size_t parentIndex;
    int parentDefined;
    Position position;
    float gScore, hScore;

} GridNode;

int grid[GRID_Y][GRID_X] = {
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 1, 1, 1, 1, 1, 1},
};

float distance(const Position *p1, const Position *p2)
{
    int dx = p1->x - p2->x;
    int dy = p1->y - p2->y;
    return sqrt(dx * dx + dy * dy);
}

DSA *getWalkableNeighbours(const Position *position)
{
    /* Returns DSA<Position> */
    DSA *positions = dsa_create(sizeof(position), 8);
    if (position->x != 0)
    {
        if (WALKABLE(position->x - 1, position->y))
        {
            dsa_add(positions, &(Position){position->x - 1, position->y});
        }
        if (position->y != 0 && WALKABLE(position->x - 1, position->y - 1))
        {
            dsa_add(positions, &(Position){position->x - 1, position->y - 1});
        }
        if (position->y != GRID_Y - 1 && WALKABLE(position->x - 1, position->y + 1))
        {
            dsa_add(positions, &(Position){position->x - 1, position->y + 1});
        }
    }
    if (position->x != GRID_X - 1)
    {
        if (WALKABLE(position->x + 1, position->y))
        {
            dsa_add(positions, &(Position){position->x + 1, position->y});
        }
        if (position->y != 0 && WALKABLE(position->x + 1, position->y - 1))
        {
            dsa_add(positions, &(Position){position->x + 1, position->y - 1});
        }
        if (position->y != GRID_Y - 1 && WALKABLE(position->x + 1, position->y + 1))
        {
            dsa_add(positions, &(Position){position->x + 1, position->y + 1});
        }
    }

    if (position->y != 0 && WALKABLE(position->x, position->y - 1))
    {
        dsa_add(positions, &(Position){position->x, position->y - 1});
    }

    if (position->y != GRID_Y - 1 && WALKABLE(position->x, position->y + 1))
    {
        dsa_add(positions, &(Position){position->x, position->y + 1});
    }
    return positions;
}

int positionInDSA(DSA *nodes, DSA *openOrClosed, const Position *searchVal, size_t *indexInOpenOrClosed, size_t *indexInNodes)
{
    for (size_t i = 0; i<openOrClosed->length; i++)
    {
        if (POSITION_EQUAL(DSA_INDEX_AS(nodes, DSA_INDEX_AS(openOrClosed, i, size_t), GridNode).position, *searchVal))
        {
            *indexInOpenOrClosed = i;
            *indexInNodes = DSA_INDEX_AS(openOrClosed, i, size_t);
            return 1;
        }
    }
    return 0;
}

size_t getLowestNodeWFScore(DSA *nodes, DSA *open)
{
    /* Returns index from open with lowest F score */
    size_t lowest = 0;
    GridNode *lowestNode = (GridNode*)DSA_INDEX_TO_P(nodes, DSA_INDEX_AS(open, 0, size_t));
    for (size_t i = 1; i<open->length; i++)
    {
        GridNode *checkNode = (GridNode*)DSA_INDEX_TO_P(nodes, DSA_INDEX_AS(open, i, size_t));
        if (F_SCORE(*checkNode) < F_SCORE(*lowestNode))
        {
            lowest = i;
            lowestNode = (GridNode*)DSA_INDEX_TO_P(nodes, DSA_INDEX_AS(open, lowest, size_t));

        }
        else if (F_SCORE(*checkNode) == F_SCORE(*lowestNode))
        {
            if (checkNode->hScore < lowestNode->hScore)
            {
                lowest = i;
                lowestNode = (GridNode*)DSA_INDEX_TO_P(nodes, DSA_INDEX_AS(open, lowest, size_t));

            }
        }
    }
    return lowest;
}

void printPathInGrid(DSA *nodes, const Position *start, const Position *dest, size_t currentNodeIndex)
{
    DSA *output = dsa_create(sizeof(char), GRID_X * GRID_Y * 2+1);
        for (int y = 0; y<GRID_Y; y++)
        {
            for (int x = 0; x<GRID_Y; x++)
            {
                if (WALKABLE(x, y))
                {
                    dsa_add(output, &(char){'*'});
                }
                else
                {
                    dsa_add(output, &(char){'|'});
                }
                if (x != GRID_Y - 1)
                {
                    dsa_add(output, &(char){' '});
                }
            }
            dsa_add(output, &(char){'\n'});
        }
        while (1)
        {
            GridNode curretNode = DSA_INDEX_AS(nodes, currentNodeIndex, GridNode);
            size_t outputIndex = (curretNode.position.y * GRID_X * 2) + (curretNode.position.x * 2);
            dsa_replace(output, outputIndex, &(char){'x'});
            if (!curretNode.parentDefined)
            {
                break;
            }
            currentNodeIndex = curretNode.parentIndex;
        }
        dsa_replace(output, (start->y * GRID_X * 2) + (start->x * 2), &(char){'S'});
        dsa_replace(output, (dest->y * GRID_X * 2) + (dest->x * 2), &(char){'D'});
        dsa_add(output, &(char){'\0'});
        printf("%s", (char*)output->data);
        dsa_free(output);
}

void a_star(const Position *start, const Position *dest)
{
    DSA *nodes = dsa_create(sizeof(GridNode), (int)distance(start, dest));
    DSA *open = dsa_create(sizeof(size_t), DSA_INITIAL_ELEMENT_COUNT);
    DSA *closed = dsa_create(sizeof(size_t), DSA_INITIAL_ELEMENT_COUNT);

    GridNode startNode = {.position = {start->x, start->y}};
    startNode.hScore = distance(start, dest);
    dsa_add(nodes, &startNode);
    dsa_add(open, &(int){0});

    size_t currentNodeIndex = 0;
    while (open->length != 0)
    {
        size_t currentNodeIndexInOpen = getLowestNodeWFScore(nodes, open);
        currentNodeIndex = DSA_INDEX_AS(open, currentNodeIndexInOpen, size_t);
        if (POSITION_EQUAL(DSA_INDEX_AS(nodes, currentNodeIndex, GridNode).position, *dest))
        {
            break;
        }

        DSA *successsorsPositions = getWalkableNeighbours(&DSA_INDEX_AS(nodes, currentNodeIndex, GridNode).position);

        for (int i = 0; i<successsorsPositions->length; i++)
        {
            float successorCurrentCost = DSA_INDEX_AS(nodes, currentNodeIndex, GridNode).gScore 
                + distance(&DSA_INDEX_AS(nodes, currentNodeIndex, GridNode).position, &DSA_INDEX_AS(successsorsPositions, i, Position));
            size_t successorNodeIndex = 0;
            size_t successorNodeIndexInOpenOrClosed = 0;
            /* If successor in open */
            if (positionInDSA(nodes, open, &DSA_INDEX_AS(successsorsPositions, i, Position), &successorNodeIndexInOpenOrClosed, &successorNodeIndex))
            {
                if (DSA_INDEX_AS(nodes, successorNodeIndex, GridNode).gScore <= successorCurrentCost)
                {
                    continue;
                }
            }
            else if (positionInDSA(nodes, closed, &DSA_INDEX_AS(successsorsPositions, i, Position), &successorNodeIndexInOpenOrClosed, &successorNodeIndex))
            {
                if (DSA_INDEX_AS(nodes, successorNodeIndex, GridNode).gScore <= successorCurrentCost)
                {
                    continue;
                }
                dsa_add(open, &DSA_INDEX_AS(closed, successorNodeIndexInOpenOrClosed, size_t));
                successorNodeIndexInOpenOrClosed = open->length-1;
                dsa_remove(closed, successorNodeIndexInOpenOrClosed);
            }
            else
            {
                
                GridNode successorNode;
                successorNode.position.x = DSA_INDEX_AS(successsorsPositions, i, Position).x;
                successorNode.position.y = DSA_INDEX_AS(successsorsPositions, i, Position).y;
                successorNode.hScore = distance(&DSA_INDEX_AS(successsorsPositions, i, Position), dest);
                dsa_add(nodes, &successorNode);
                successorNodeIndex = nodes->length-1;
                dsa_add(open, &successorNodeIndex);
                successorNodeIndexInOpenOrClosed = open->length-1;
            }
            DSA_INDEX_AS(nodes, successorNodeIndex, GridNode).parentDefined = 1;
            DSA_INDEX_AS(nodes, successorNodeIndex, GridNode).parentIndex = currentNodeIndex;
        }
        dsa_add(closed, &currentNodeIndex);
        dsa_remove(open, currentNodeIndexInOpen);
        dsa_free(successsorsPositions);
    }

    if (!POSITION_EQUAL(DSA_INDEX_AS(nodes, currentNodeIndex, GridNode).position, *dest))
    {
        printf("Not Found !");
    }
    else
    {
        printPathInGrid(nodes, start, dest, currentNodeIndex);
    }
    dsa_free(nodes);
    dsa_free(open);
    dsa_free(closed);
}

int main()
{
    Position start = {0, 0};
    Position dest = {5, 5};
    a_star(&start, &dest);
}