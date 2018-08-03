# -*- coding: utf-8 -*-

extensions = [ 'sphinx.ext.imgmath' ,
'sphinx.ext.autodoc',
'sphinx.ext.intersphinx',
'autoapi.extension',
'IPython.sphinxext.ipython_console_highlighting',
'IPython.sphinxext.ipython_directive',
'breathe',
'exhale',
'm2r'
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
html_logo = 'in_images/logo_sirius.jpg'
html_search_language = 'en'
#html_sidebars = { '**': ['globaltoc.html', 'relations.html', 'sourcelink.html', 'searchbox.html']}
htmlhelp_basename = 'Sirius'

# -- Options for LaTeX output ---------------------------------------------
latex_documents = [(master_doc, 'Sirius.tex', u'sirius_doc', u'CS', 'manual')]
#latex_logo = None


# -- Options for exhale / breath -----------------------------------------
# Setup the breathe extension
breathe_projects = {
    "Sirius": "doxy_xml"
}
breathe_default_project = "Sirius"

# Setup the exhale extension
exhale_args = {
    # These arguments are required
    "containmentFolder":     "api",
    "rootFileName":          "library_root.rst",
    "rootFileTitle":         "Library API",
    "doxygenStripFromPath":  ".",
    # Suggested optional arguments
    "createTreeView":        True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    "exhaleExecutesDoxygen": False
#    "exhaleDoxygenStdin":    "INPUT = ../../../src/sirius/"
}
# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'
# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'
