#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

using jsoncons::ojson;
using std::istream;
using std::ifstream;
namespace jsonschema = jsoncons::jsonschema;

static bool CheckJson(istream &schemaFile, istream &jsonFile, std::string_view jsonFileName) {
  ojson schema = ojson::parse(schemaFile);
  jsonschema::json_schema<ojson> compiledSchema = jsonschema::make_json_schema(
    std::move(schema)
  );

  ojson json;
  uint_fast32_t errorCount = 0;
  auto reporter = [&errorCount](const jsonschema::validation_message& message) -> jsonschema::walk_result {
          ++errorCount;
          std::cout << message.instance_location().string() << ": " << message.message() << std::endl;
          return jsonschema::walk_result::advance;
  };

  std::cout << "Parsing " << jsonFileName << std::endl;

  try {
      json = ojson::parse(jsonFile);
  } catch (const jsoncons::ser_error& e) {
      std::cout << "Parsing failed: " << e.what() << std::endl;
      return false;
  }

  std::cout << "Validating " << jsonFileName << std::endl;
  compiledSchema.validate(json, reporter);
  std::cout << "Validation complete, " << errorCount << " errors found" << std::endl;

  return true;
}

int main(void) {
  std::istringstream metaSchema(R"(
{
    "$schema": "https://json-schema.org/draft/2019-09/schema"
}
    )");

  ifstream schemaFile("GameData.schema.json");
  ifstream gameDataFile("GameData.json");

  if (!CheckJson(metaSchema, schemaFile, "schema")) {
    goto end;
  }
  schemaFile.clear();
  schemaFile.seekg(0, std::ios_base::beg);

  CheckJson(schemaFile, gameDataFile, "game data");

end:
  gameDataFile.close();
  schemaFile.close();
}
