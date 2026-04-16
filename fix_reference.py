from docx import Document
from docx.shared import Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH

doc = Document("custom-reference.docx")

style_config = {
    "Title": {"font_size": 22, "bold": True, "alignment": WD_ALIGN_PARAGRAPH.CENTER},
    "Heading 1": {"font_size": 16, "bold": True, "space_before": 18, "space_after": 10},
    "Heading 2": {"font_size": 14, "bold": True, "space_before": 14, "space_after": 8},
    "Heading 3": {"font_size": 12, "bold": True, "space_before": 10, "space_after": 6},
    "Normal": {"font_size": 11, "space_after": 6, "line_spacing": 1.5},
    "First Paragraph": {"font_size": 11, "space_after": 6, "line_spacing": 1.5},
}

for name, cfg in style_config.items():
    try:
        style = doc.styles[name]
    except KeyError:
        continue
    pf = style.paragraph_format
    rf = style.font
    rf.size = Pt(cfg["font_size"])
    rf.name = "Times New Roman"
    if cfg.get("bold"):
        rf.bold = True
    if cfg.get("alignment"):
        pf.alignment = cfg["alignment"]
    if cfg.get("space_before"):
        pf.space_before = Pt(cfg["space_before"])
    if cfg.get("space_after"):
        pf.space_after = Pt(cfg["space_after"])
    if cfg.get("line_spacing"):
        pf.line_spacing = cfg["line_spacing"]

# Set CJK font via element manipulation
from docx.oxml.ns import qn
for name in style_config:
    try:
        style = doc.styles[name]
    except KeyError:
        continue
    rpr = style.element.find(qn("w:rPr"))
    if rpr is None:
        from lxml import etree
        rpr = etree.SubElement(style.element, qn("w:rPr"))
    rfonts = rpr.find(qn("w:rFonts"))
    if rfonts is None:
        from lxml import etree
        rfonts = etree.SubElement(rpr, qn("w:rFonts"))
    rfonts.set(qn("w:eastAsia"), "宋体")

# Page margins
for section in doc.sections:
    section.top_margin = Cm(2.54)
    section.bottom_margin = Cm(2.54)
    section.left_margin = Cm(3.17)
    section.right_margin = Cm(3.17)

doc.save("custom-reference.docx")
print("Reference template updated.")
