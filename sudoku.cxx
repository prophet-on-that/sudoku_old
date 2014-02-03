// TODO const points in vectors??
// TODO should use iterators as opposed to array access to vectors

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <algorithm> // erase-remove
#include <cstdlib> // exit
#include <iterator> // ostream_iterator

using namespace std;

typedef vector<int> Cell;
typedef vector<Cell> Row;
typedef vector<Row> Grid;
typedef pair<int, int> Point; // (row, column)

const int SIZE = 9;
const int SUBGRID_SIZE = 3;

void 
die (const string error)
{
  cout << error << endl;
  exit (1);
}

Grid 
read ()
{
  Grid grid;
  string buffer;
  vector<int> unknown_cell;
  for (int i = 0; i < SIZE; ++i)
    unknown_cell.push_back (i + 1);

  for (int i = 0; i < SIZE; ++i)
  {
    Row row;
    getline (cin, buffer);
    for (
      string::const_iterator cit = buffer.begin (); 
      cit != buffer.end (); 
      ++cit
    )
    {
      int entry = static_cast<int> (*cit) - 48;    
      if (entry < 0 || entry > SIZE)
        die ("Invalid character in input");
      if (entry)
        row.push_back (Cell (1, entry));
      else 
        row.push_back (unknown_cell);
    }
    if (row.size () != SIZE)
      die ("Invalid row length in input");
    grid.push_back (row);
  } 
  return grid;
}

// Pre: point is a known cell in grid
// Removes the known value from all cells in the same row, column and subgrid
vector<Point>
update (Grid &grid, const Point point)
{
  // Generate list of cells to update
  vector<Point> to_update;
  // Add cells in row
  for (int j = 0; j < SIZE; j++)
    if (j == point.second)
      continue;
    else
      to_update.push_back (Point (point.first, j));
  // Add cells in column
  for (int i = 0; i < SIZE; i++)
    if (i == point.first) 
      continue; 
    else
      to_update.push_back (Point (i, point.second));
  // Add remaining cells in subgrid
  Point subgrid = Point (
    point.first / SUBGRID_SIZE, point.second / SUBGRID_SIZE
  );
  Point offset = Point (
    point.first % SUBGRID_SIZE, point.second % SUBGRID_SIZE
  );
  for (int i = 0; i < SUBGRID_SIZE; i++)
  {
    if (i == offset.first)
      continue;
    for (int j = 0; j < SUBGRID_SIZE; j++)
    {
      if (j == offset.second)
        continue;
      to_update.push_back (
        Point (
          SUBGRID_SIZE * subgrid.first + i, 
          SUBGRID_SIZE * subgrid.second + j
        )
      );
    }
  }
  // delete from each cell
  int to_remove = grid[point.first][point.second][0];
  vector<Point> completed;
  for (
    vector<Point>::const_iterator cit = to_update.begin (); 
    cit != to_update.end (); 
    cit++
  )
  {
    vector<int> &v = grid[cit->first][cit->second];
    if (v.size () > 1)
    {
      v.erase (remove (v.begin (), v.end (), to_remove), v.end ());
      if (v.size () == 1)
        completed.push_back (*cit);
    }
  }

  return completed;
}

// Given a row, return a list of pairs (unassigned value, positions in row that value can
// occupy)
vector<pair <int, vector<int> > >
possibilities (const Row &row)
{
  vector< vector<int> > possibilities; // Possibilities for each value 1..9
  for (int i = 0; i < SIZE; i++)
    possibilities.push_back (vector<int> ());
  for (
    Row::const_iterator cit = row.begin ();
    cit != row.end ();
    cit++
  )
  {
    int index = cit - row.begin ();
    for (
      Cell::const_iterator cjt = cit->begin ();
      cjt != cit->end ();
      cjt++
    )
      possibilities[*cjt - 1].push_back (index);
  }
  // Build into list of pairs
  vector <pair <int, vector<int> > > ret;
  for (
    vector< vector<int> >::const_iterator cit = possibilities.begin ();
    cit != possibilities.end ();
    cit++
  )
  {
    // add if value unassigned
    int value = 1 + cit - possibilities.begin ();
    if (cit->size () > 1)
      ret.push_back (pair<int, vector<int> > (value, *cit));
  }
  return ret;
}

vector< pair<Point, int> >
find_necessary_assignments (const Grid &grid)
{
  vector< pair<Point, int> > necessary; 
  // Rows
  for (int i = 0; i < SIZE; i++)
  {
    vector<pair <int, vector<int> > > poss = possibilities (grid[i]);
    for (
      vector<pair <int, vector<int> > >::const_iterator cit = poss.begin ();
      cit != poss.end ();
      cit++
    )
      if (cit->second.size () == 1)
      {
        // modify and add to update list
        int value = cit->first;
        int position = cit->second.front ();
        necessary.push_back (pair<Point, int> (Point (i, position), value));
      }
  }
  // Columns
  for (int j = 0; j < SIZE; j++)
  {
    // Aggreate column as temp row
    Row row;
    for (int i = 0; i < SIZE; i++)
      row.push_back (grid[i][j]);
    
    vector<pair <int, vector<int> > > poss = possibilities (row);
    for (
      vector<pair <int, vector<int> > >::const_iterator cit = poss.begin ();
      cit != poss.end ();
      cit++
    )
      if (cit->second.size () == 1)
      {
        // modify and add to update list
        int value = cit->first;
        int position = cit->second.front ();
        necessary.push_back (pair<Point, int> (Point (position, j), value));
      }
  }
  // Subgrid // TODO clean
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    {
      // Aggregate subgrid as temp row
      Row row;
      for (int i_offset = 0; i < 3; i++)
        for (int j_offset = 0; j < 3; j++)
          row.push_back (grid[3*i + i_offset][3*j + i_offset]);
      vector<pair <int, vector<int> > > poss = possibilities (row);
      for (
        vector<pair <int, vector<int> > >::const_iterator cit = poss.begin ();
        cit != poss.end ();
        cit++
      )
        if (cit->second.size () == 1)
        {
          // modify and add to update list
          int value = cit->first;
          int position = cit->second.front ();
          Point offset = Point (position / 3, position % 3);
          Point coord = Point (3 * i + offset.first, 3 * j + offset.second);
          necessary.push_back (pair<Point, int> (coord, value));
        }
    }
  return necessary;
}

vector<Point>
completed_cells (const Grid &grid)
{
  vector<Point> ret;
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      if (grid[i][j].size () == 1)
        ret.push_back (Point (i, j));
  return ret;
}

bool
complete (const Grid &grid)
{
  return completed_cells (grid).size () == SIZE * SIZE;
}

void 
print (const Grid &grid)
{
  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
      cout << grid[i][j][0];
    cout << endl;
  }
}

// For debugging
void
print_incomplete (const Grid &grid)
{
  for (int i = 0; i < SIZE; i++)
  {
    for (int j = 0; j < SIZE; j++)
    {
      const Cell &cell = grid[i][j];
      if (cell.size () == 1)
        cout << cell[0];
      else 
      {
        cout << "{";
        copy (
          cell.begin (), 
          cell.end (), 
          ostream_iterator<int> (cout, ",")
        );
        cout << "}";
      }
      cout << " ";    
    }
    cout << endl;
  }
}

int
main (int argc, char **argv)
{
  Grid grid = read ();
  vector<Point> update_queue = completed_cells (grid); 
  for (;;)
  {
    if (update_queue.empty ())
    {
      if (complete (grid))
        break;
      else 
      {
        vector< pair<Point, int> > necessary = find_necessary_assignments (grid);
        if (!necessary.size ())
          die ("Stuck!");
        // adjust grid accordingly
        cout << "USING COOL STUFF";
        for (
          vector< pair<Point, int> >::const_iterator cit = necessary.begin ();
          cit != necessary.end ();
          cit++
        )
        {
          const Point coord = cit->first;
          const int value = cit->second;
          grid[coord.first][coord.second].clear ();
          grid[coord.first][coord.second].push_back (value);
          update_queue.push_back (coord);
        }
      }
    }
    vector<Point> to_update = update (grid, update_queue.front ());
    update_queue.erase (update_queue.begin ());
    update_queue.insert (update_queue.end (), to_update.begin (), to_update.end ());
  }
  print (grid);
}
