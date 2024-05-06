#include <cstdint>
#include <fstream>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

using jsoncons::ojson;
namespace jsonschema = jsoncons::jsonschema;

int main() {
  std::ifstream schemaFile("GameData.schema.json");
  std::ifstream gameDataFile("GameData.json");

  ojson schema = ojson::parse(schemaFile);
  jsonschema::json_schema<ojson> compiledSchema = jsonschema::make_json_schema(std::move(schema));

  ojson gameData;
  uint_fast32_t errorCount = 0;
  auto reporter = [&errorCount](const jsonschema::validation_message& message) -> jsonschema::walk_result {
          ++errorCount;
          std::cout << message.instance_location().string() << ": " << message.message() << std::endl;
          return jsonschema::walk_result::advance;
  };

  std::cout << "Parsing game data" << std::endl;

  try {
      gameData = ojson::parse(gameDataFile);
  } catch (const jsoncons::ser_error& e) {
      std::cout << "Parsing failed: " << e.what() << std::endl;
      goto end;
  }

  std::cout << "Validating game data" << std::endl;
  compiledSchema.validate(gameData, reporter);
  std::cout << "Validation complete, " << errorCount << " errors found" << std::endl;

end:
  schemaFile.close();
}
