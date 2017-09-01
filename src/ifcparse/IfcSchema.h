/********************************************************************************
 *                                                                              *
 * This file is part of IfcOpenShell.                                           *
 *                                                                              *
 * IfcOpenShell is free software: you can redistribute it and/or modify         *
 * it under the terms of the Lesser GNU General Public License as published by  *
 * the Free Software Foundation, either version 3.0 of the License, or          *
 * (at your option) any later version.                                          *
 *                                                                              *
 * IfcOpenShell is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * Lesser GNU General Public License for more details.                          *
 *                                                                              *
 * You should have received a copy of the Lesser GNU General Public License     *
 * along with this program. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                              *
 ********************************************************************************/

#ifndef IFCSCHEMA_H
#define IFCSCHEMA_H

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

#include <boost/algorithm/string.hpp>

#include "../ifcparse/IfcException.h"

#ifdef USE_IFC4
#include "../ifcparse/Ifc4enum.h"
#else
#include "../ifcparse/Ifc2x3enum.h"
#endif

namespace IfcParse {

	class declaration; 
	
	class type_declaration; 
	class select_type; 
	class enumeration_type; 
	class entity;

	class named_type;
	class simple_type;
	class aggregation_type;

	class parameter_type {
	public:
		virtual const named_type* as_named_type() const { return static_cast<named_type*>(0); }
		virtual const simple_type* as_simple_type() const { return static_cast<simple_type*>(0); }
		virtual const aggregation_type* as_aggregation_type() const { return static_cast<aggregation_type*>(0); }

		virtual bool is(const std::string& /*name*/) const { return false; }
		virtual bool is(IfcSchema::Type::Enum /*name*/) const { return false; }
	};

	class named_type : public parameter_type {
	protected:
		declaration* declared_type_;
	public:
		named_type(declaration* declared_type)
			: declared_type_(declared_type) {}

		declaration* declared_type() const { return declared_type_; }

		virtual const named_type* as_named_type() const { return this; }

		virtual bool is(const std::string& name) const;
		virtual bool is(IfcSchema::Type::Enum name) const;
	};

	class simple_type : public parameter_type {
	public:
		typedef enum { binary_type, boolean_type, integer_type, logical_type, number_type, real_type, string_type, datatype_COUNT } data_type;
	protected:
		data_type declared_type_;
	public:
		simple_type(data_type declared_type)
			: declared_type_(declared_type) {}

		data_type declared_type() const { return declared_type_; }

		virtual const simple_type* as_simple_type() const { return this; }
	};

	class aggregation_type : public parameter_type {
	public:
		typedef enum { array_type, bag_type, list_type, set_type } aggregate_type;
	protected:
		aggregate_type type_of_aggregation_;
		int bound1_, bound2_;
		parameter_type* type_of_element_;
	public:
		aggregation_type(aggregate_type type_of_aggregation, int bound1, int bound2, parameter_type* type_of_element)
			: type_of_aggregation_(type_of_aggregation)
			, bound1_(bound1)
			, bound2_(bound2)
			, type_of_element_(type_of_element)
		{}

		aggregate_type type_of_aggregation() const { type_of_aggregation_; }
		int bound1() const { return bound1_; }
		int bound2() const { return bound2_; }
		parameter_type* type_of_element() const { return type_of_element_; }

		virtual const aggregation_type* as_aggregation_type() const { return this; }
	};

	class declaration {
	protected:
		// std::string name_;
		IfcSchema::Type::Enum name_;

	public:
		declaration(IfcSchema::Type::Enum name)
			: name_(name) {}
		declaration(const std::string& name)
			: name_(IfcSchema::Type::FromString(name)) {}

		std::string name() const { return IfcSchema::Type::ToString(name_); }

		virtual const type_declaration* as_type_declaration() const { return static_cast<type_declaration*>(0); }
		virtual const select_type* as_select_type() const { return static_cast<select_type*>(0); }
		virtual const enumeration_type* as_enumeration_type() const { return static_cast<enumeration_type*>(0); }
		virtual const entity* as_entity() const { return static_cast<entity*>(0); }

		// TODO: Type checking by Enum value
		bool is(const std::string& name) const;
		bool is(IfcSchema::Type::Enum name) const;

		IfcSchema::Type::Enum type() const {
			return name_;
		}
	};

	class type_declaration : public declaration {
	protected:
		const parameter_type* declared_type_;

	public:
		type_declaration(const std::string& name, const parameter_type* declared_type)
			: declaration(name)
			, declared_type_(declared_type)	{}
		type_declaration(IfcSchema::Type::Enum name, const parameter_type* declared_type)
			: declaration(name)
			, declared_type_(declared_type)	{}

		const parameter_type* declared_type() const { return declared_type_; }

		virtual const type_declaration* as_type_declaration() const { return this; }
	};

	class select_type : public declaration {
	protected:
		std::vector<const declaration*> select_list_;
	public:
		select_type(const std::string& name, const std::vector<const declaration*>& select_list)
			: declaration(name)
			, select_list_(select_list)	{}
		select_type(IfcSchema::Type::Enum name, const std::vector<const declaration*>& select_list)
			: declaration(name)
			, select_list_(select_list)	{}

		const std::vector<const declaration*>& select_list() const { return select_list_; }

		virtual const select_type* as_select_type() const { return this; }
	};

	class enumeration_type : public declaration {
	protected:
		std::vector<std::string> enumeration_items_;
	public:
		enumeration_type(const std::string& name, const std::vector<std::string>& enumeration_items)
			: declaration(name)
			, enumeration_items_(enumeration_items)	{}
		enumeration_type(IfcSchema::Type::Enum name, const std::vector<std::string>& enumeration_items)
			: declaration(name)
			, enumeration_items_(enumeration_items)	{}

		const std::vector<std::string>& enumeration_items() const { return enumeration_items_; }

		virtual const enumeration_type* as_enumeration_type() const { return this; }
	};

	class entity : public declaration {
	public:
		class attribute {
		protected:
			std::string name_;
			const parameter_type* type_of_attribute_;
			bool optional_;

		public:
			attribute(const std::string& name, parameter_type* type_of_attribute, bool optional)
				: name_(name)
				, type_of_attribute_(type_of_attribute)
				, optional_(optional) {}

			const std::string& name() const { return name_; }
			const parameter_type* type_of_attribute() const { return type_of_attribute_; }
			bool optional() const { return optional_; }
		};

		class inverse_attribute {
		public:
			typedef enum { bag_type, set_type, unspecified_type } aggregate_type;
		protected:
			std::string name_;
			aggregate_type type_of_aggregation_;
			int bound1_, bound2_;
			parameter_type* type_of_element_;
			const entity* entity_reference_;
			const attribute* attribute_reference_;
		public:
			inverse_attribute(const std::string& name, aggregate_type type_of_aggregation, int bound1, int bound2, const entity* entity_reference, const attribute* attribute_reference)
				: name_(name)
				, type_of_aggregation_(type_of_aggregation)
				, bound1_(bound1)
				, bound2_(bound2)
				, entity_reference_(entity_reference)
				, attribute_reference_(attribute_reference)
			{}

			const std::string& name() const { return name_; }
			aggregate_type type_of_aggregation() const { type_of_aggregation_; }
			int bound1() const { return bound1_; }
			int bound2() const { return bound2_; }
			const entity* entity_reference() const { return entity_reference_; }
			const attribute* attribute_reference() const { return attribute_reference_; }
		};
		
	protected:
		const entity* supertype_; /* NB: IFC explicitly allows only single inheritance */
		std::vector<const entity*> subtypes_;

		std::vector<const attribute*> attributes_;
		std::vector<bool> derived_;

		std::vector<const inverse_attribute*> inverse_attributes_;

		class attribute_by_name_cmp : public std::unary_function<const attribute*, bool> {
		private:
			std::string name_;
		public:
			attribute_by_name_cmp(const std::string name)
				: name_(name) {}
			bool operator()(const attribute* attr) {
				return attr->name() == name_;
			}
		};
	public:
		entity(const std::string& name, entity* supertype)
			: declaration(name)
			, supertype_(supertype)
		{}
		entity(IfcSchema::Type::Enum name, entity* supertype)
			: declaration(name)
			, supertype_(supertype)
		{}

		bool is(const std::string& name) const {
			return is(IfcSchema::Type::FromString(name));
		}

		bool is(IfcSchema::Type::Enum name) const {
			if (name == name_) return true;
			else if (supertype_) return supertype_->is(name);
			else return false;
		}

		void set_subtypes(const std::vector<const entity*>& subtypes) {
			subtypes_ = subtypes;
		}

		void set_attributes(const std::vector<const attribute*>& attributes, const std::vector<bool>& derived) {
			attributes_ = attributes;
			derived_ = derived;
		}

		void set_inverse_attributes(const std::vector<const inverse_attribute*>& inverse_attributes) {
			inverse_attributes_ = inverse_attributes;
		}

		const std::vector<const entity*>& subtypes() const { return subtypes_; }
		const std::vector<const attribute*>& attributes() const { return attributes_; }
		const std::vector<bool>& derived() const { return derived_; }
	
		const std::vector<const attribute*> all_attributes() const {
			std::vector<const attribute*> attrs;
			attrs.reserve(derived_.size());
			if (supertype_) {
				const std::vector<const attribute*> supertype_attrs = supertype_->all_attributes();
				std::copy(supertype_attrs.begin(), supertype_attrs.end(), std::back_inserter(attrs));
			}
			std::copy(attributes_.begin(), attributes_.end(), std::back_inserter(attrs));
			return attrs;
		}

		const std::vector<const inverse_attribute*> all_inverse_attributes() const {
			std::vector<const inverse_attribute*> attrs;
			if (supertype_) {
				const std::vector<const inverse_attribute*> supertype_inv_attrs = supertype_->all_inverse_attributes();
				std::copy(supertype_inv_attrs.begin(), supertype_inv_attrs.end(), std::back_inserter(attrs));
			}
			std::copy(inverse_attributes_.begin(), inverse_attributes_.end(), std::back_inserter(attrs));
			return attrs;
		}

		ptrdiff_t attribute_index(const attribute* attr) const {
			const entity* current = this;
			ptrdiff_t index = -1;
			do {
				if (index > -1) {
					index += current->attributes().size();
				} else {
					auto it = std::find(current->attributes().begin(), current->attributes().end(), attr);
					if (it != current->attributes().end()) {
						index = std::distance(current->attributes().begin(), it);
					}
				}
			} while (current = current->supertype_);
			return index;
		}

		ptrdiff_t attribute_index(const std::string& attr_name) const {
			const entity* current = this;
			ptrdiff_t index = -1;
			attribute_by_name_cmp cmp(attr_name);
			do {
				if (index > -1) {
					index += current->attributes().size();
				} else {
					auto it = std::find_if(current->attributes().begin(), current->attributes().end(), cmp);
					if (it != current->attributes().end()) {
						index = std::distance(current->attributes().begin(), it);
					}
				}
			} while (current = current->supertype_);
			return index;
		}

		const entity* supertype() const { return supertype_; }

		virtual const entity* as_entity() const { return this; }
	};

	class schema_definition {
	private:
		bool built_in_;

		std::string name_;

		std::vector<const declaration*> declarations_;

		std::vector<const type_declaration*> type_declarations_;
		std::vector<const select_type*> select_types_;
		std::vector<const enumeration_type*> enumeration_types_;
		std::vector<const entity*> entities_;

		class declaration_by_name_cmp : public std::binary_function<const declaration*, const std::string&, bool> {
		public:
			bool operator()(const declaration* decl, const std::string& name) {
				// TODO: Efficiency?
				return boost::to_lower_copy(decl->name()) < boost::to_lower_copy(name);
			}
		};

		class declaration_by_enum_cmp : public std::binary_function<const declaration*, IfcSchema::Type::Enum, bool> {
		public:
			bool operator()(const declaration* decl, IfcSchema::Type::Enum name) {
				return decl->type() < name;
			}
		};

		class declaration_by_enum_sort : public std::binary_function<const declaration*, const declaration*, bool> {
		public:
			bool operator()(const declaration* a, const declaration* b) {
				return a->type() < b->type();
			}
		};

	public:
		schema_definition(const std::string& name, const std::vector<const declaration*>& declarations, const bool built_in = false)
			: name_(name)
			, declarations_(declarations)
			, built_in_(built_in)
		{
			std::sort(declarations_.begin(), declarations_.end(), declaration_by_enum_sort());
			for (std::vector<const declaration*>::const_iterator it = declarations_.begin(); it != declarations_.end(); ++it) {
				if ((**it).as_type_declaration()) type_declarations_.push_back((**it).as_type_declaration());
				if ((**it).as_select_type()) select_types_.push_back((**it).as_select_type());
				if ((**it).as_enumeration_type()) enumeration_types_.push_back((**it).as_enumeration_type());
				if ((**it).as_entity()) entities_.push_back((**it).as_entity());
			}			
		}

		~schema_definition() {
			for (std::vector<const declaration*>::const_iterator it = declarations_.begin(); it != declarations_.end(); ++it) {
				delete *it;
			}
		}

		const declaration* declaration_by_name(const std::string& name) const {
			std::vector<const declaration*>::const_iterator it = std::lower_bound(declarations_.begin(), declarations_.end(), name, declaration_by_name_cmp());
			if (it == declarations_.end() || boost::to_lower_copy((**it).name()) != boost::to_lower_copy(name)) {
				throw IfcParse::IfcException("Entity with '" + name + "' not found");
			} else {
				return *it;
			}
		}

		const declaration* declaration_by_name(IfcSchema::Type::Enum name) const {
			if (!built_in_) throw;
			return declarations_[name];
		}

		const std::vector<const declaration*>& declarations() const { return declarations_; }
		const std::vector<const type_declaration*>& type_declarations() const { return type_declarations_; }
		const std::vector<const select_type*>& select_types() const { return select_types_; }
		const std::vector<const enumeration_type*>& enumeration_types() const { return enumeration_types_; }
		const std::vector<const entity*>& entities() const { return entities_; }

		const std::string& name() const { return name_; }
	};

}

#endif
