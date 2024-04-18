CXX = g++ -std=c++17
DIR = /Library/Frameworks
LIBS = -F$(DIR) -framework Gecode

nlsp: nlsp.cpp
	$(CXX) -F$(DIR) -o nlsp nlsp.cpp $(LIBS)

clean:
	rm -f nlsp

run_instances:
	@./run.sh

run_and_save:
	@./run_save_out.sh


