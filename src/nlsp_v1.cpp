#include <iostream>
#include <vector>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <bitset>
#include <cmath>

using namespace std;

class Nlsp : public Gecode::Space
{
protected:
  int num_variables;
  int num_leaves;
  vector<pair<vector<int>, int>> truth_table;
  Gecode::IntVarArray leaves;

public:
  Nlsp(vector<pair<vector<int>, int>> truth_table, int num_variables, int num_leaves) : num_variables(num_variables), num_leaves(num_leaves), truth_table(truth_table), leaves(*this, num_variables * num_leaves, 0, 1)
  {
    for (int _idx = 0; _idx < truth_table.size(); _idx++)
    {
      vector<int> xs = truth_table[_idx].first;
      int y = truth_table[_idx].second;

      // Result will be between 0 and 1 inclusive
      Gecode::IntVarArray results(*this, num_leaves, 0, 1);
      // Define integer coefficients
      Gecode::IntArgs coefficients(num_variables);
      for (int idx = 0; idx < num_variables; idx++)
      {
        coefficients[idx] = xs[idx];
      }

      for (int i = 0; i < num_leaves; i++)
      {
        Gecode::IntVarArgs values(*this, num_variables, 0, 1);
        for (int j = 0; j < num_variables; j++)
        {
          values[j] = get_value(i, j);
        }
        // Define integer expression representing c1 * x_1 + c2 * x_2
        // Perform the operation: c1 * x1 + c2 * x2
        Gecode::linear(*this, coefficients, values, Gecode::IRT_EQ, results[i]);
      }

      // N=2l−1 = 2*4-1 = 7 -> NOR-nodes = 7-4 = 3
      // [-1, -1, -1, 0, 0, 0, 0]
      // a complete binary tree represented for a heap
      Gecode::BoolVarArray nodes(*this, 2 * num_leaves - 1, 0, 1);

      for (int _i = nodes.size() - num_leaves; _i < nodes.size(); _i++)
      {
        int value_idx = abs(nodes.size() - num_leaves - _i);
        Gecode::rel(*this, nodes[_i] == (results[value_idx] > 0));
      }

      Gecode::rel(*this, nodes[0], Gecode::IRT_EQ, y);

      for (int idx = 0; idx < nodes.size() - num_leaves; idx++)
      {
        // Parent: (current index - 1) // 2 (round down)
        int left_idx = idx * 2 + 1;  // Left child: (current index * 2) + 1
        int right_idx = idx * 2 + 2; // Right child: (current index * 2) + 2

        // Enforce that the current node is the NOR of its children
        Gecode::rel(*this, nodes[idx] == !(nodes[left_idx] || nodes[right_idx]));
      }
    }

    Gecode::branch(*this, leaves, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
    // Gecode::branch(*this, leaves, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MAX());
  }

  Gecode::IntVar get_value(int row, int column) const
  {
    return leaves[row * num_variables + column];
  }

  int NOR(int x1, int x2)
  {
    return !(x1 || x2);
  }

  Nlsp(Nlsp &s) : Gecode::Space(s)
  {
    leaves.update(*this, s.leaves);
    num_leaves = s.num_leaves;
    num_variables = s.num_variables;
    truth_table = s.truth_table;
  }

  virtual Gecode::Space *copy()
  {
    return new Nlsp(*this);
  }

  void print() const
  {
    cout << num_variables << endl;
    for (int _idx = 0; _idx < truth_table.size(); _idx++)
    {
      int y = truth_table[_idx].second;
      cout << y << endl;
    }
    // [-1, -1, -1, 0, 0, 0, 0] -> total number of nodes = N = 2l−1
    int num_nodes = 2 * num_leaves - 1;
    // Depth of a complete binary tree can be calculated using logarithmic formula -> depth = Log base 2 of (number of nodes)
    int d = log2(num_nodes); // Depth of the circuit
    //  Number of NOR nodes -> NOR-nodes = (2l−1)-l = l - 1 (aka size)
    int s = num_leaves - 1; // Number of NOR nodes in the circuit (size)

    cout << d << " " << s << endl;
    for (int i = 0; i < s; i++)
    {
      // <id> <code> <left> <right>
      int id = i + 1;
      int left_id = (i * 2 + 1) + 1;  // Left child: (current index * 2) + 1
      int right_id = (i * 2 + 2) + 1; // Right child: (current index * 2) + 2
      cout << id << " -1"
           << " " << left_id << " " << right_id << endl;
    }
    for (int i = 0; i < num_leaves; i++)
    {
      int elem = 0;
      for (int j = 0; j < num_variables; j++)
      {
        Gecode::IntVar value = get_value(i, j);
        if (value.val() == 1)
        {
          elem = j + 1;
        }
      }
      // <id> <code> <left> <right>
      int id = num_leaves + i;
      cout << id << " " << elem << " 0 0 " << endl;
    }
  }
};

vector<int> Binary(int num, int base)
{
  vector<int> v;
  // Binary representation of num with base bits
  // Start from the leftmost bit (most significant bit)
  for (int i = base - 1; i >= 0; i--)
  {
    // Use bitwise AND operation to check if the i-th bit is set
    int bit = (num >> i) & 1;
    v.push_back(bit);
  }
  return v;
}

int main()
{
  // vector<int> v({0, 0, 0, 1});
  // int vars = 2;
  // vector<int> v({0, 1, 1, 0, 0, 1, 1, 0});
  int vars;
  cin >> vars;
  int rows = pow(2, vars);
  /*
  In a complete binary tree, where every level is completely filled except possibly for the last level (which is often used as the basis for a heap), the number of nodes can be calculated based on the number of leaves.
  For a binary tree with l leaves:
  The total number of nodes (N) can be calculated using the formula:
  N=2l−1
  */
  vector<pair<vector<int>, int>> truth_table;
  // bitset<2> binary(i);
  // Convert the integer to binary

  for (int i = 0; i < rows; i++)
  {
    int _temp;
    cin >> _temp;
    truth_table.push_back({Binary(i, vars), _temp});
  }

  int l = 2;
  while (true)
  {
    Nlsp *m = new Nlsp(truth_table, vars, l);
    Gecode::DFS<Nlsp> e(m);
    delete m;
    if (Nlsp *s = e.next())
    {
      s->print();
      delete s;
      break; // Found a solution, exit the loop
    }
    else
    {
      l++; // Increment l if no feasible solution found
    }
  }
  return 0;
}