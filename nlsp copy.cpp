#include <iostream>
#include <vector>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <bitset>

using namespace std;

class Nlsp : public Gecode::Space
{
protected:
  int num_variables;
  int num_leaves;
  Gecode::IntVarArray m_l;

public:
  Nlsp(vector<int> y, int num_variables, int num_leaves) : num_variables(num_variables), num_leaves(num_leaves), m_l(*this, num_variables * num_leaves, 0, 1)
  {

    for (int i = 0; i < y.size(); i++)
    {
      int elem = y[i];
      std::bitset<2> binary(i);
      std::cout << "Nlsp constructor, i: " << i << " value: " << elem << " binary: " << binary << std::endl;
      // N=2l−1 = 2*4-1 = 7 -> NOR-nodes = 7-4 = 3
      // [-1, -1, -1, 0, 0, 0, 0]

      Gecode::BoolVar _aux_left(*this, 0, 1);
      Gecode::BoolVar _aux_right(*this, 0, 1);
      Gecode::BoolVar result(*this, 0, 1);
      // Result will be between 0 and 1 inclusive
      Gecode::IntVarArray values(*this, num_leaves, 0, 1);

      // Define integer coefficients
      Gecode::IntArgs c({binary[0], binary[1]});
      for (int j = 0; j < num_leaves; j++)
      {
        Gecode::IntVarArgs v({get_value(j, 0), get_value(j, 1)});
        // Define integer expression representing c1 * x_1 + c2 * x_2
        Gecode::linear(*this, c, v, Gecode::IRT_EQ, values[j]);
      }
      Gecode::rel(*this, _aux_left == !(values[0] + values[1] > 0));  // _aux_left = !(values[0] || values[1])
      Gecode::rel(*this, _aux_right == !(values[2] + values[3] > 0)); // _aux_right = !(values[2] || values[3])
      Gecode::rel(*this, result == !(_aux_left || _aux_right));
      Gecode::rel(*this, result, Gecode::IRT_EQ, elem);
    }

    Gecode::branch(*this, m_l, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
    // Gecode::branch(*this, m_l, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MAX());
  }

  Gecode::IntVar get_value(int row, int column) const
  {
    return m_l[row * num_variables + column];
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

  void NOR_constraint(Gecode::BoolVarArray values, Gecode::BoolVar y)
  {
    Gecode::BoolVar _aux_left(*this, 0, 1);
    Gecode::BoolVar _aux_right(*this, 0, 1);

    Gecode::rel(*this, _aux_left == !(values[0] || values[1]));
    Gecode::rel(*this, _aux_right == !(values[2] || values[3]));
    Gecode::rel(*this, y == !(_aux_left || _aux_right));
  }
  Nlsp(Nlsp &s) : Gecode::Space(s)
  {
    m_l.update(*this, s.m_l);
  }

  virtual Gecode::Space *copy()
  {
    return new Nlsp(*this);
  }

  void print(void) const
  {
    cout << "Nlsp print" << endl;
    for (int i = 0; i < m_l.size(); i += 2)
    {
      int elem1 = m_l[i].val();
      int elem2 = m_l[i + 1].val();
      if (elem1 == 0 && elem2 == 0)
      {
        cout << "0 ";
      }
      else if (elem1 == 1 && elem2 == 0)
      {
        cout << "1 ";
      }
      else if (elem1 == 0 && elem2 == 1)
      {
        cout << "2 ";
      }
      else
      {
        cout << "x ";
      }
    }
    cout << endl;
  }
};

int main()
{
  vector<int> v({0, 0, 0, 1});
  int max_xs = 2;
  /*
  In a complete binary tree, where every level is completely filled except possibly for the last level (which is often used as the basis for a heap), the number of nodes can be calculated based on the number of leaves.
  For a binary tree with l leaves:
  The total number of nodes (N) can be calculated using the formula:
  N=2l−1
  */
  int l = 4;
  Nlsp *m = new Nlsp(v, max_xs, l);
  Gecode::DFS<Nlsp> e(m);
  delete m;

  while (Nlsp *s = e.next())
  {
    s->print();
    delete s;
  }
  return 0;
}