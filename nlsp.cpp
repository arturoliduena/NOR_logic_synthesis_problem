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
  Gecode::IntVarArray leaves;

public:
  Nlsp(vector<int> y, int num_variables, int num_leaves) : num_variables(num_variables), num_leaves(num_leaves), leaves(*this, num_variables * num_leaves, 0, 1)
  {
    std::cout << "Nlsp constructor, num_leaves: " << num_leaves << " num_variables: " << num_variables << std::endl;

    for (int i = 0; i < y.size(); i++)
    {
      int elem = y[i];
      // std::bitset<2> binary(i);
      // Convert the integer to binary
      vector<int> binary = Binary(i, num_variables);
      // Result will be between 0 and 1 inclusive
      Gecode::IntVarArray values(*this, num_leaves, 0, 1);
      // Define integer coefficients
      // Gecode::IntArgs c({binary[0], binary[1]});
      Gecode::IntArgs c(num_variables);
      for (int k = 0; k < num_variables; k++)
      {
        c[k] = binary[k];
      }
      for (int j = 0; j < num_leaves; j++)
      {
        Gecode::IntVarArgs v(*this, num_variables, 0, 1);
        for (int k = 0; k < num_variables; k++)
        {
          v[k] = get_value(j, k);
        }
        // Define integer expression representing c1 * x_1 + c2 * x_2
        // Perform the operation: c1 * x1 + c2 * x2
        // where c1, c2, x1, and x2 are boolean values
        // Equivalent expression: (c1 && x1) || (c2 && x2)

        Gecode::linear(*this, c, v, Gecode::IRT_EQ, values[j]);
      }

      // N=2l−1 = 2*4-1 = 7 -> NOR-nodes = 7-4 = 3
      // [-1, -1, -1, 0, 0, 0, 0]
      // a complete binary tree represented for a heap
      Gecode::BoolVarArray nodes(*this, 2 * num_leaves - 1, 0, 1);

      for (int i = nodes.size() - num_leaves; i < nodes.size(); i++)
      {
        int value_idx = std::abs(nodes.size() - num_leaves - i);
        Gecode::rel(*this, nodes[i] == (values[value_idx] > 0));
      }

      Gecode::rel(*this, nodes[0], Gecode::IRT_EQ, elem);

      for (int idx = 0; idx < nodes.size() - num_leaves; idx++)
      {
        // Parent: (current index - 1) // 2 (round down)
        int left_idx = idx * 2 + 1;  // Left child: (current index * 2) + 1
        int right_idx = idx * 2 + 2; // Right child: (current index * 2) + 2
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

  vector<int> Binary(int x, int c)
  {
    vector<int> v;
    // Binary representation of x with c bits
    // Start from the leftmost bit (most significant bit)
    for (int i = c - 1; i >= 0; i--)
    {
      // Use bitwise AND operation to check if the i-th bit is set
      int bit = (x >> i) & 1;
      v.push_back(bit);
    }
    return v;
  }

  int NOR(int x1, int x2)
  {
    return !(x1 || x2);
  }

  void NOR_c(Gecode::BoolVar x1, Gecode::BoolVar x2, Gecode::BoolVar y)
  {
    // Enforce that y is the NOR of x1 and x2
    Gecode::rel(*this, y == !(x1 || x2));
  }

  Nlsp(Nlsp &s) : Gecode::Space(s)
  {
    leaves.update(*this, s.leaves);
    num_leaves = s.num_leaves;
    num_variables = s.num_variables;
  }

  virtual Gecode::Space *copy()
  {
    return new Nlsp(*this);
  }

  void print(void) const
  {
    for (int j = 0; j < num_leaves; j++)
    {
      int elem = 0;
      for (int k = 0; k < num_variables; k++)
      {
        Gecode::IntVar value = get_value(j, k);
        if (value.val() == 1)
        {
          elem = k;
        }
      }
      cout << elem << " ";
    }
    cout << endl;
  }
};

int main()
{
  // vector<int> v({0, 0, 0, 1});
  // int max_xs = 2;
  vector<int> v({0, 1, 1, 0, 0, 1, 1, 0});
  int max_xs = 3;
  /*
  In a complete binary tree, where every level is completely filled except possibly for the last level (which is often used as the basis for a heap), the number of nodes can be calculated based on the number of leaves.
  For a binary tree with l leaves:
  The total number of nodes (N) can be calculated using the formula:
  N=2l−1
  */
  int l = 2;
  while (true)
  {
    Nlsp *m = new Nlsp(v, max_xs, l);
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