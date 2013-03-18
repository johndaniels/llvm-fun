
#include <string>
#include <vector>
#include <memory>

namespace lang {
	class Assignment {
	public:
		Assignment(std::shared_ptr<std::string> id, int value) {
			this->id = id;
			this->value = value;
		}
		std::shared_ptr<std::string> id;
		int value;
	};
	typedef std::shared_ptr<Assignment> AssignmentP;

	class CompilationUnit {
	public:
		CompilationUnit(std::shared_ptr<std::vector<std::shared_ptr<Assignment> > > statements) : statements(statements) {}
		std::shared_ptr<std::vector<std::shared_ptr<Assignment> > > statements;
	};
	typedef std::shared_ptr<CompilationUnit> CompilationUnitP;

	std::shared_ptr<CompilationUnit> parse();
}