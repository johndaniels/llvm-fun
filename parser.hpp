
#include <memory>
#include <string>
#include <vector>

namespace lang {
	class Assignment {
	public:
		Assignment(const std::string& id, int value) {
			this->id = id;
			this->value = value;
		}
		std::string id;
		int value;
	};

	class Ast {
	public:
		std::shared_ptr<std::vector<std::shared_ptr<Assignment> > > statements;
	};

	std::shared_ptr<Ast> parse();
}