# Configuration file for the Sphinx documentation builder.
# grm: Telegram CLI client powered by TDLib and C++23

project = 'grm'
copyright = '2026, Rénich Bon Ćirić (Copyleft)'
author = 'Rénich Bon Ćirić'
release = '0.9.1'

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.todo',
]

templates_path = ['_templates']
exclude_patterns = ['_build', 'build', 'Thumbs.db', '.DS_Store']

try:
    import sphinx_rtd_theme
    html_theme = 'sphinx_rtd_theme'
except ImportError:
    html_theme = 'alabaster'

html_static_path = []
html_title = 'grm Documentation'
html_show_sourcelink = False
