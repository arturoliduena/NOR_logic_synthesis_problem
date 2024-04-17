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
  int depth;
  int num_nodes;
  vector<pair<vector<int>, int>> truth_table;
  Gecode::IntVarArray nodes;
  // node is_NOR {1: NOR, 0: variable}
  Gecode::BoolVarArray is_NOR;

public:
  Nlsp(vector<pair<vector<int>, int>> truth_table, int num_variables, int depth) : num_variables(num_variables), depth(depth), truth_table(truth_table), num_nodes(pow(2, depth + 1) - 1), nodes(*this, num_nodes * num_variables, 0, 1), is_NOR(*this, num_nodes, 0, 1)
  {
    // root node should be a NOR node, so the type of the node should be 1
    Gecode::rel(*this, is_NOR[0] == 1);

    for (int i = 0; i < num_nodes; i++)
    {
      Gecode::IntVarArgs vars(*this, num_variables, 0, 1);
      for (int j = 0; j < num_variables; j++)
      {
        vars[j] = get_value(i, j);
      }

      Gecode::IntVar sum_vars(*this, 0, num_variables);
      Gecode::linear(*this, vars, Gecode::IRT_EQ, sum_vars);
      Gecode::rel(*this, sum_vars, Gecode::IRT_EQ, 0, Gecode::imp(is_NOR[i]));
      // Gecode::rel(*this, (is_NOR[i] == 1) >> (sum_vars == 0));
      // Gecode::rel(*this, (types[i] == 1) << (sum_vars > 0));

      // Parent: (current index - 1) // 2 (round down)
      int left_idx = i * 2 + 1;  // Left child: (current index * 2) + 1
      int right_idx = i * 2 + 2; // Right child: (current index * 2) + 2

      // Enforce that the current node is the NOR of its children
      if (left_idx < num_nodes && right_idx < num_nodes)
      {
        // Gecode::rel(*this, is_NOR[i], Gecode::IRT_EQ, 1);
      }
      else
      {
        // If the node is a leaf node cant't be a NOR node
        Gecode::rel(*this, is_NOR[i], Gecode::IRT_NQ, 1);
      }
    }

    for (int _idx = 0; _idx < truth_table.size(); _idx++)
    {
      vector<int> xs = truth_table[_idx].first;
      int y = truth_table[_idx].second;

      // node value will be between 0 and 1 inclusive
      Gecode::IntVarArray values(*this, num_nodes, 0, 1);

      // Define integer coefficients
      Gecode::IntArgs coefficients(num_variables);
      for (int idx = 0; idx < num_variables; idx++)
      {
        coefficients[idx] = xs[idx];
      }

      for (int i = 0; i < num_nodes; i++)
      {
        Gecode::IntVarArgs vars(*this, num_variables, 0, 1);
        for (int j = 0; j < num_variables; j++)
        {
          vars[j] = get_value(i, j);
        }
        // Define integer expression representing c1 * x_1 + c2 * x_2
        Gecode::linear(*this, coefficients, vars, Gecode::IRT_EQ, values[i]);
      }

      // a binary tree represented for a heap
      Gecode::BoolVarArray tree(*this, num_nodes, 0, 1);

      Gecode::rel(*this, tree[0], Gecode::IRT_EQ, y);

      for (int idx = 0; idx < tree.size(); idx++)
      {
        // Parent: (current index - 1) // 2 (round down)
        int left_idx = idx * 2 + 1;  // Left child: (current index * 2) + 1
        int right_idx = idx * 2 + 2; // Right child: (current index * 2) + 2

        if (left_idx < tree.size() && right_idx < tree.size())
        {
          Gecode::rel(*this, (tree[idx] == !(tree[left_idx] || tree[right_idx])) << (is_NOR[idx] == 1));
        }
        Gecode::rel(*this, (tree[idx] == values[idx]) << (is_NOR[idx] == 0));
      }
    }

    Gecode::branch(*this, nodes, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
    Gecode::branch(*this, is_NOR, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MAX());
  }

  Gecode::IntVar get_value(int row, int column) const
  {
    return nodes[row * num_variables + column];
  }

  Nlsp(Nlsp &s) : Gecode::Space(s)
  {
    nodes.update(*this, s.nodes);
    is_NOR.update(*this, s.is_NOR);
    num_nodes = s.num_nodes;
    num_variables = s.num_variables;
    truth_table = s.truth_table;
    depth = s.depth;
  }

  virtual Gecode::Space *copy()
  {
    return new Nlsp(*this);
  }

  void print_matrix() const
  {
    cout << "_____________MATRIX___________________" << endl;
    for (int i = 0; i < num_nodes; i++)
    {
      for (int j = 0; j < num_variables; j++)
      {
        Gecode::IntVar value = get_value(i, j);
        cout << value.val() << " ";
      }
      cout << endl;
    }
  }

  void print_node_types() const
  {
    cout << "______________node-types__________________" << endl;
    for (int i = 0; i < num_nodes; i++)
    {
      if (is_NOR[i].assigned())
      {
        cout << is_NOR[i].val() << " ";
      }
      else
      {
        cout << "- ";
      }
    }
    cout << endl;
  }

  void print() const
  {
    cout << num_variables << endl;
    for (int idx = 0; idx < truth_table.size(); idx++)
    {
      int y = truth_table[idx].second;
      cout << y << endl;
    }
    int s = 0; // Number of NOR nodes in the circuit (size)
    for (int i = 0; i < num_nodes; i++)
    {
      if (is_NOR[i].val() == 1)
      {
        s++;
      }
    }
    cout << depth << " " << s << endl;

    for (int i = 0; i < num_nodes; i++)
    {
      // <id> <code> <left> <right>
      int id = i + 1;

      if (is_NOR[i].val() == 1)
      {
        int left_id = (i * 2 + 1) + 1;  // Left child: (current index * 2) + 1
        int right_id = (i * 2 + 2) + 1; // Right child: (current index * 2) + 2
        cout << id << " -1"
             << " " << left_id << " " << right_id << endl;
      }
      else
      {
        int elem = 0;
        // Parent: (current index - 1) // 2 (round down)
        int const parent_idx = (i - 1) / 2;
        if (is_NOR[parent_idx].val() == 1)
        {
          for (int j = 0; j < num_variables; j++)
          {
            Gecode::IntVar value = get_value(i, j);
            if (value.val() == 1)
            {
              elem = j + 1;
            }
          }
          cout << id << " " << elem << " 0 0 " << endl;
        }
      }
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
  for (int i = 0; i < rows; i++)
  {
    int _temp;
    cin >> _temp;
    truth_table.push_back({Binary(i, vars), _temp});
  }

  int depth = 1;
  while (true)
  {
    Nlsp *m = new Nlsp(truth_table, vars, depth);
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
      depth++; // Increment depth if no feasible solution found
    }
  }
  return 0;
}