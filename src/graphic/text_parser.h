/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_GRAPHIC_TEXT_PARSER_H
#define WL_GRAPHIC_TEXT_PARSER_H

#include <string>
#include <vector>

#include "graphic/align.h"
#include "graphic/color.h"

namespace UI {

/**
 * Corresponds to a richtext block that is enclosed in <p></p> tags.
 * Has uniform font style, contains text pre-split into words, and keeps track of
 * manual line breaks (<br>) in a separate structure.
 */
struct TextBlock {
	TextBlock();
	// Copy and assignement operators are autogenerated.

	void set_font_size(int32_t const font_size) {
		font_size_ = font_size;
	}
	int32_t get_font_size() const {
		return font_size_;
	}

	void set_font_color(const RGBColor& font_color) {
		font_color_ = font_color;
	}
	RGBColor get_font_color() const {
		return font_color_;
	}

	void set_font_weight(const std::string& font_weight) {
		font_weight_ = font_weight;
	}
	const std::string& get_font_weight() const {
		return font_weight_;
	}

	void set_font_style(const std::string& font_style) {
		font_style_ = font_style;
	}
	const std::string& get_font_style() const {
		return font_style_;
	}

	void set_font_decoration(const std::string& font_decoration) {
		font_decoration_ = font_decoration;
	}
	const std::string& get_font_decoration() const {
		return font_decoration_;
	}

	void set_font_face(const std::string& font_face) {
		font_face_ = font_face;
	}
	const std::string& get_font_face() const {
		return font_face_;
	}

	void set_line_spacing(int32_t const line_spacing) {
		line_spacing_ = line_spacing;
	}
	int32_t get_line_spacing() const {
		return line_spacing_;
	}

	void set_words(const std::vector<std::string>& words) {
		words_ = words;
	}
	const std::vector<std::string>& get_words() const {
		return words_;
	}

	void set_line_breaks(const std::vector<std::vector<std::string>::size_type>& line_breaks) {
		line_breaks_ = line_breaks;
	}
	const std::vector<std::vector<std::string>::size_type>& get_line_breaks() const {
		return line_breaks_;
	}

private:
	int32_t font_size_;
	RGBColor font_color_;
	std::string font_weight_;
	std::string font_style_;
	std::string font_decoration_;
	std::string font_face_;
	int32_t line_spacing_;
	std::vector<std::string> words_;

	/**
	 * Position of manual line breaks (<br>) with respect to @ref words_.
	 * Sorted in ascending order.
	 * An entry j in this vector means that a manual line break occurs
	 * before the j-th word in @ref words_. In particular, an entry 0
	 * means that a manual line break occurs before the first word.
	 * Entries can appear with multiplicity, indicating that multiple
	 * manual line breaks exist without any words in-between.
	 */
	std::vector<std::vector<std::string>::size_type> line_breaks_;
};

struct RichtextBlock {
	RichtextBlock();
	RichtextBlock(const RichtextBlock& src);

	void set_images(const std::vector<std::string>& images) {
		images_ = images;
	}
	const std::vector<std::string>& get_images() const {
		return images_;
	}

	void set_image_align(Align const image_align) {
		image_align_ = image_align;
	}
	Align get_image_align() const {
		return image_align_;
	}

	void set_text_align(Align const text_align) {
		text_align_ = text_align;
	}
	Align get_text_align() const {
		return text_align_;
	}

	void set_text_blocks(const std::vector<TextBlock>& text_blocks) {
		text_blocks_ = text_blocks;
	}
	const std::vector<TextBlock>& get_text_blocks() const {
		return text_blocks_;
	}

private:
	std::vector<std::string> images_;
	std::vector<TextBlock> text_blocks_;
	Align image_align_;
	Align text_align_;
};

struct TextParser {
	void parse(std::string& text, std::vector<RichtextBlock>& blocks);

private:
	void parse_richtexttext_attributes(std::string format, RichtextBlock*);
	bool parse_textblock(std::string& block,
	                     std::string& block_format,
	                     std::vector<std::string>& words,
	                     std::vector<std::vector<std::string>::size_type>& line_breaks);
	void parse_text_attributes(std::string format, TextBlock&);
	bool extract_format_block(std::string& block,
	                          std::string& block_text,
	                          std::string& block_format,
	                          const std::string& block_start,
	                          const std::string& format_end,
	                          const std::string& block_end);
	Align set_align(const std::string&);
	void split_words(const std::string& in, std::vector<std::string>* plist);
};
}

#endif  // end of include guard: WL_GRAPHIC_TEXT_PARSER_H
