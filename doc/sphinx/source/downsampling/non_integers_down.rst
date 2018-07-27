.. include:: ../common.rst

.. _non_int_down:

*******************************
Non integer downsampling factor
*******************************

Sirius way to handle non integer downsampling factor consists in two step :

* Upsampling to a common sampling grid (between input and output sampling grids)
* Decimating to the output sampling grid

To illustrate this, say the input spacing is :math:`{in}_{s}` and output one is :math:`{out}_{s}` with
:math:`{out}_{s} > {in}_{s}, \frac{{out}_{s}}{{in}_{s}} \notin \mathbb N^{*}`, then the downsampling factor deduced by
Sirius is :math:`z_{down} = \frac{p}{q} = \frac{{out}_{s}}{{in}_{s}}` with :math:`(p,q) \in \mathbb N^{*2}`. The first
step is then to upsample the input signal by a factor :math:`q`. Then, the output is decimated so that only :math:`1`
pixel out of :math:`p` is kept.

.. ipython:: python
    :suppress:

    input_sampling = 2
    output_sampling = 3
    common_grid = np.linspace(0,6,7)
    plt.plot(common_grid[::2],np.ones(common_grid[::2].size)*3,'x',label='Input sampling grid');
    plt.legend(bbox_to_anchor=(0.6, 0.4), bbox_transform=plt.gcf().transFigure);
    plt.plot(common_grid,np.ones(common_grid.size)*2,'x',label='Intermediate sampling grid');
    plt.legend(bbox_to_anchor=(0.6, 0.4), bbox_transform=plt.gcf().transFigure);
    @savefig grid_sampling.png
    plt.plot(common_grid[::3],np.ones(common_grid[::3].size)*1,'x',label='Output sampling grid'); plt.legend(bbox_to_anchor=(0.6, 0.4), bbox_transform=plt.gcf().transFigure); plt.tick_params(axis='y', which='both', left=False, right=False, labelleft=False)

