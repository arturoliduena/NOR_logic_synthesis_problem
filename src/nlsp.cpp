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
  Gecode::BoolVarArray nodes;
  // node is_NOR {1: NOR, 0: variable}
  Gecode::BoolVarArray is_NOR;

public:
  Nlsp(vector<pair<vector<int>, int>> truth_table, int num_variables, int depth) : num_variables(num_variables), depth(depth), truth_table(truth_table), num_nodes(pow(2, depth + 1) - 1), nodes(*this, num_nodes * num_variables, 0, 1), is_NOR(*this, num_nodes, 0, 1)
  {
    for (int i = 0; i < num_nodes; i++)
    {
      Gecode::BoolVarArgs vars(*this, num_variables, 0, 1);
      for (int j = 0; j < num_variables; j++)
      {
        vars[j] = get_value(i, j);
      }

      Gecode::linear(*this, vars, Gecode::IRT_LQ, 1);
      Gecode::linear(*this, vars, Gecode::IRT_EQ, 0, Gecode::imp(is_NOR[i]));

      // Parent: (current index - 1) // 2 (round down)
      int left_idx = i * 2 + 1;  // Left child: (current index * 2) + 1
      int right_idx = i * 2 + 2; // Right child: (current index * 2) + 2

      if (!(left_idx < num_nodes && right_idx < num_nodes))
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
        Gecode::BoolVarArgs vars(*this, num_variables, 0, 1);
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

        Gecode::BoolVar value(*this, 0, 1);
        Gecode::channel(*this, values[idx], value);
        if (left_idx < tree.size() && right_idx < tree.size())
        {
          Gecode::ite(*this, is_NOR[idx], NOR(tree[left_idx], tree[right_idx]), value, tree[idx]);
        }
        else
        {
          Gecode::rel(*this, tree[idx], Gecode::IRT_EQ, value);
        }
      }
    }

    Gecode::branch(*this, nodes, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MAX());
    Gecode::branch(*this, is_NOR, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MAX());
  }

  Gecode::BoolVar get_value(int row, int column) const
  {
    return nodes[row * num_variables + column];
  }

  Gecode::BoolVar NOR(Gecode::BoolVar x1, Gecode::BoolVar x2)
  {
    Gecode::BoolVar result = Gecode::expr(*this, !(x1 || x2));
    return result;
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

  virtual void constrain(const Space &_b)
  {
    const Nlsp &b = static_cast<const Nlsp &>(_b);
    int s = 0; // Number of NOR nodes in the circuit (size)
    for (int i = 0; i < num_nodes; i++)
    {
      if (b.is_NOR[i].val() == 1)
      {
        s++;
      }
    }
    Gecode::linear(*this, is_NOR, Gecode::IRT_LE, s);
  }

  void print_matrix() const
  {
    cout << "_____________MATRIX___________________" << endl;
    for (int i = 0; i < num_nodes; i++)
    {
      for (int j = 0; j < num_variables; j++)
      {
        Gecode::BoolVar value = get_value(i, j);
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
            Gecode::BoolVar value = get_value(i, j);
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
  vector<pair<vector<int>, int>> truth_table;
  for (int i = 0; i < rows; i++)
  {
    int _temp;
    cin >> _temp;
    truth_table.push_back({Binary(i, vars), _temp});
  }

  int depth = 0;
  while (true)
  {
    Nlsp *m = new Nlsp(truth_table, vars, depth);
    Gecode::DFS<Nlsp> e(m);
    delete m;
    if (Nlsp *s = e.next())
    {
      Nlsp *_s = e.next();
      while (_s != NULL)
      {
        delete s;
        s = _s;
        _s = e.next();
      }
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