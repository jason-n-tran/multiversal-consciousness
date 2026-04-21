#include "LevelLoader.h"
#include "PuzzleValidator.h"
#include <fstream>
#include <sstream>
#include <iostream>

LevelLoader::LevelLoader(EntityManager* entity_manager, ComponentRegistry* component_registry)
    : entity_manager_(entity_manager), component_registry_(component_registry) {
    puzzle_validator_ = std::make_unique<PuzzleValidator>(entity_manager, component_registry);
}

LevelData LevelLoader::load_level_from_file(const std::string& filename) {
    LevelData level_data;
    
    std::cout << "Opening file: " << filename << std::endl;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file: " << filename << std::endl;
        return level_data;
    }
    
    std::cout << "File opened successfully" << std::endl;
    
    std::string line;
    std::string current_section;
    std::stringstream section_data;
    
    while (std::getline(file, line)) {
        std::cout << "Read line: " << line << std::endl;
        
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        if (line[0] == '[' && line.back() == ']') {
            std::cout << "Found section header: " << line << std::endl;
            
            if (!current_section.empty()) {
                std::cout << "Processing section: " << current_section << std::endl;
                if (current_section == "info") {
                    std::cout << "Processing info section" << std::endl;
                    std::string info_data = section_data.str();
                    std::cout << "Info data: " << info_data << std::endl;
                    size_t name_pos = info_data.find("name=");
                    if (name_pos != std::string::npos) {
                        size_t start = name_pos + 5;
                        size_t end = info_data.find('\n', start);
                        if (end == std::string::npos) {
                            end = info_data.length();
                        }
                        level_data.name = info_data.substr(start, end - start);
                        std::cout << "Parsed name: " << level_data.name << std::endl;
                    }
                    
                    size_t desc_pos = info_data.find("description=");
                    if (desc_pos != std::string::npos) {
                        size_t start = desc_pos + 12;
                        size_t end = info_data.find('\n', start);
                        if (end == std::string::npos) {
                            end = info_data.length();
                        }
                        level_data.description = info_data.substr(start, end - start);
                        std::cout << "Parsed description: " << level_data.description << std::endl;
                    }
                } else if (current_section == "agents") {
                    level_data.agents = parse_agents(section_data.str());
                } else if (current_section == "quantum_nodes") {
                    level_data.quantum_nodes = parse_quantum_nodes(section_data.str());
                } else if (current_section == "environment") {
                    level_data.environment = parse_environment(section_data.str());
                } else if (current_section == "conditions") {
                    level_data.completion_conditions = parse_conditions(section_data.str());
                }
            }
            
            current_section = line.substr(1, line.length() - 2);
            section_data.str("");
            section_data.clear();
        } else {
            section_data << line << "\n";
        }
    }
    
    if (!current_section.empty()) {
        if (current_section == "agents") {
            level_data.agents = parse_agents(section_data.str());
        } else if (current_section == "quantum_nodes") {
            level_data.quantum_nodes = parse_quantum_nodes(section_data.str());
        } else if (current_section == "environment") {
            level_data.environment = parse_environment(section_data.str());
        } else if (current_section == "conditions") {
            level_data.completion_conditions = parse_conditions(section_data.str());
        }
    }
    
    file.close();
    std::cout << "Loaded level: " << level_data.name << std::endl;
    return level_data;
}

std::vector<LevelAgent> LevelLoader::parse_agents(const std::string& data) {
    std::vector<LevelAgent> agents;
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        LevelAgent agent;
        std::istringstream line_stream(line);
        std::string token;
        
        if (std::getline(line_stream, token, ',')) {
            agent.agent_type = token;
        }
        if (std::getline(line_stream, token, ',')) {
            agent.agent_number = static_cast<uint8_t>(std::stoi(token));
        }
        if (std::getline(line_stream, token, ',')) {
            agent.x = std::stof(token);
        }
        if (std::getline(line_stream, token, ',')) {
            agent.y = std::stof(token);
        }
        if (std::getline(line_stream, token, ',')) {
            agent.movement_speed = std::stof(token);
        }
        
        agents.push_back(agent);
    }
    
    return agents;
}

std::vector<LevelQuantumNode> LevelLoader::parse_quantum_nodes(const std::string& data) {
    std::vector<LevelQuantumNode> nodes;
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        LevelQuantumNode node;
        std::istringstream line_stream(line);
        std::string token;
        
        if (std::getline(line_stream, token, ',')) {
            node.x = std::stof(token);
        }
        if (std::getline(line_stream, token, ',')) {
            node.y = std::stof(token);
        }
        if (std::getline(line_stream, token, ',')) {
            node.reality_a_item = token;
        }
        if (std::getline(line_stream, token, ',')) {
            node.reality_b_item = token;
        }
        if (std::getline(line_stream, token, ',')) {
            node.interaction_radius = std::stof(token);
        }
        
        nodes.push_back(node);
    }
    
    return nodes;
}

std::vector<LevelEnvironment> LevelLoader::parse_environment(const std::string& data) {
    std::vector<LevelEnvironment> environment;
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        LevelEnvironment env;
        std::istringstream line_stream(line);
        std::string token;
        
        if (std::getline(line_stream, token, ',')) {
            env.type = token;
        }
        if (std::getline(line_stream, token, ',')) {
            env.x = std::stof(token);
        }
        if (std::getline(line_stream, token, ',')) {
            env.y = std::stof(token);
        }
        
        while (std::getline(line_stream, token, ',')) {
            size_t eq_pos = token.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = token.substr(0, eq_pos);
                std::string value = token.substr(eq_pos + 1);
                env.properties[key] = value;
            }
        }
        
        environment.push_back(env);
    }
    
    return environment;
}

std::vector<PuzzleCondition> LevelLoader::parse_conditions(const std::string& data) {
    std::vector<PuzzleCondition> conditions;
    std::istringstream stream(data);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        PuzzleCondition condition;
        std::istringstream line_stream(line);
        std::string token;
        
        if (std::getline(line_stream, token, ',')) {
            condition.type = token;
        }
        if (std::getline(line_stream, token, ',')) {
            condition.target = token;
        }
        if (std::getline(line_stream, token, ',')) {
            condition.value = token;
        }
        
        conditions.push_back(condition);
    }
    
    return conditions;
}

std::vector<EntityID> LevelLoader::instantiate_level(const LevelData& level_data) {
    std::vector<EntityID> created_entities;
    
    if (!entity_manager_ || !component_registry_) {
        std::cerr << "LevelLoader not properly initialized" << std::endl;
        return created_entities;
    }
    
    for (const auto& agent_data : level_data.agents) {
        EntityID entity = create_agent(agent_data);
        if (entity != 0) {
            created_entities.push_back(entity);
        }
    }
    
    for (const auto& node_data : level_data.quantum_nodes) {
        EntityID entity = create_quantum_node(node_data);
        if (entity != 0) {
            created_entities.push_back(entity);
        }
    }
    
    for (const auto& env_data : level_data.environment) {
        EntityID entity = create_environment(env_data);
        if (entity != 0) {
            created_entities.push_back(entity);
        }
    }
    
    std::cout << "Instantiated level with " << created_entities.size() << " entities" << std::endl;
    return created_entities;
}

EntityID LevelLoader::create_agent(const LevelAgent& agent_data) {
    EntityID entity = entity_manager_->create_entity();
    
    Transform transform;
    transform.x = agent_data.x;
    transform.y = agent_data.y;
    component_registry_->add_component(entity, std::move(transform));
    
    Agent agent;
    agent.agent_number = agent_data.agent_number;
    agent.movement_speed = agent_data.movement_speed;
    component_registry_->add_component(entity, std::move(agent));
    
    Renderable renderable;
    renderable.texture_id = agent_data.agent_type + "_texture";
    renderable.source_rect = {0, 0, 32, 32};
    component_registry_->add_component(entity, std::move(renderable));
    
    Inventory inventory;
    for (const auto& ability : agent_data.initial_abilities) {
        inventory.abilities[ability.first] = std::stoi(ability.second);
    }
    component_registry_->add_component(entity, std::move(inventory));
    
    return entity;
}

EntityID LevelLoader::create_quantum_node(const LevelQuantumNode& node_data) {
    EntityID entity = entity_manager_->create_entity();
    
    Transform transform;
    transform.x = node_data.x;
    transform.y = node_data.y;
    component_registry_->add_component(entity, std::move(transform));
    
    QuantumNode quantum_node;
    quantum_node.reality_a_item = node_data.reality_a_item;
    quantum_node.reality_b_item = node_data.reality_b_item;
    quantum_node.interaction_radius = node_data.interaction_radius;
    component_registry_->add_component(entity, std::move(quantum_node));
    
    Renderable renderable;
    renderable.texture_id = "quantum_node_texture";
    renderable.source_rect = {0, 0, 32, 32};
    renderable.color_r = 0.8f;
    renderable.color_g = 0.2f;
    renderable.color_b = 0.8f; // Purple color for quantum nodes
    component_registry_->add_component(entity, std::move(renderable));
    
    return entity;
}

EntityID LevelLoader::create_environment(const LevelEnvironment& env_data) {
    EntityID entity = entity_manager_->create_entity();
    
    Transform transform;
    transform.x = env_data.x;
    transform.y = env_data.y;
    component_registry_->add_component(entity, std::move(transform));
    
    if (env_data.type == "door") {
        Door door;
        auto it = env_data.properties.find("locked");
        if (it != env_data.properties.end()) {
            door.is_locked = (it->second == "true");
        }
        it = env_data.properties.find("required_key");
        if (it != env_data.properties.end()) {
            door.required_key = it->second;
        }
        component_registry_->add_component(entity, std::move(door));
        
        Renderable renderable;
        renderable.texture_id = "door_texture";
        renderable.source_rect = {0, 0, 32, 64};
        component_registry_->add_component(entity, std::move(renderable));
        
    } else if (env_data.type == "water") {
        WaterLevel water;
        auto it = env_data.properties.find("level");
        if (it != env_data.properties.end()) {
            water.current_level = std::stof(it->second);
            water.target_level = water.current_level;
        }
        component_registry_->add_component(entity, std::move(water));
        
        Renderable renderable;
        renderable.texture_id = "water_texture";
        renderable.source_rect = {0, 0, 64, 32};
        renderable.color_r = 0.2f;
        renderable.color_g = 0.6f;
        renderable.color_b = 1.0f; // Blue color for water
        component_registry_->add_component(entity, std::move(renderable));
        
    } else if (env_data.type == "switch") {
        EnvironmentalSwitch env_switch;
        auto it = env_data.properties.find("target_type");
        if (it != env_data.properties.end()) {
            env_switch.target_entity_type = it->second;
        }
        it = env_data.properties.find("target_id");
        if (it != env_data.properties.end()) {
            env_switch.target_entity_id = it->second;
        }
        component_registry_->add_component(entity, std::move(env_switch));
        
        Renderable renderable;
        renderable.texture_id = "switch_texture";
        renderable.source_rect = {0, 0, 32, 32};
        component_registry_->add_component(entity, std::move(renderable));
    }
    
    return entity;
}

bool LevelLoader::validate_puzzle_completion(const std::vector<PuzzleCondition>& conditions) {
    if (!puzzle_validator_) {
        return false;
    }
    return puzzle_validator_->are_all_conditions_met(conditions);
}

bool LevelLoader::check_condition(const PuzzleCondition& condition) {
    if (!puzzle_validator_) {
        return false;
    }
    
    ValidationResult result = puzzle_validator_->validate_condition(condition);
    return result.is_valid;
}