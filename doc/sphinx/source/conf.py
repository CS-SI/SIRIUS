# -*- coding: utf-8 -*-

extensions = [ 'sphinx.ext.imgmath' ,
'sphinx.ext.autodoc',
'sphinx.ext.intersphinx',
'autoapi.extension',
'IPython.sphinxext.ipython_console_highlighting',
'IPython.sphinxext.ipython_directive'
]

# -- General configuration ------------------------------------------------
numfig=True
source_suffix = '.rst'
source_encoding = 'utf-8'
master_doc = 'Sirius'
project = u'SIRIUS'
copyright = u'2018, CS'
author = u'CS'
language = 'en'
pygments_style = 'sphinx'
# Document Python Code
autoapi_type = 'python'
autoapi_dirs = ['', 'code_py', 'code_py']
# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']
# -- Options for HTML output ----------------------------------------------
#html_logo = 'Images/cs_cnes_200pixels.bmp'
html_theme = 'sphinx_rtd_theme'
def setup(app):
  app.add_stylesheet( "css/color_def.css" )
  app.add_stylesheet( "css/my_theme.css" )
  app.add_javascript( "custom_admonition.js" )
html_static_path = ['_static']
html_logo = '../../img/logo_sirius.jpg'
html_search_language = 'en'
#html_sidebars = { '**': ['globaltoc.html', 'relations.html', 'sourcelink.html', 'searchbox.html']}
htmlhelp_basename = 'Sirius'

# -- Options for LaTeX output ---------------------------------------------
latex_documents = [(master_doc, 'Sirius.tex', u'sirius_doc', u'CS', 'manual')]
#latex_logo = None
