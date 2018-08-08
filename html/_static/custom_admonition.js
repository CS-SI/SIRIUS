$('.gofurther').css("background-color", '#BCF5A9');
$('.gofurther > .last').hide();
$('.gofurther > .expanded + .last').show('normal');
$('.gofurther > p').click(function() {
	$(this).find('.last').hide();
	$(this).toggleClass('expanded').toggleClass('collapsed').next('.last').toggle('normal');
});
