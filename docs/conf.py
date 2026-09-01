# Configuration file for the Sphinx documentation builder.
# grm: Telegram CLI client powered by TDLib and C++23

project = 'grm'
copyright = '2026, Rénich Bon Ćirić (Copyleft)'
author = 'Rénich Bon Ćirić'
release = '0.9.0'

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'build', 'Thumbs.db', '.DS_Store']

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_title = 'grm Documentation'
html_show_sourcelink = False
