(function($, Drupal)
{
    /**
     * This function reacts to clicks of pager items
     */
    function pagerListener()
    {
        // Note: .ajax_pager_form is the class we added to our form
        // for consistency. We use this to target the pager links in this form
        // and this form only.
        $(".ajax_pager_form .pager li a").once("pager-listener", function()
        {
            // $(this) refers to the <a> tag that has been clicked.
            $(this).click(function(e)
            {
                // Prevent the link from loading a new page when clicked
                e.preventDefault();

                var matches, page;

                // Get the number of the clicked page.
                matches = $(this).attr("href").match(/page=(\d+)/);

                // Drupal pagers begin at an idex of 0, so page 1
                // has index 0, page 2 has index 1, and so on.
                // Links to the first page will not contain ?page=___
                // in the links, so there will be no matches. In this case,
                // we want to set the page value to 0.
                page = matches && matches[1] ? matches[1] : 0;

                // Insert the clicked page number into our hidden container
                $(".current_pager_container").val(page);

                // Click the submit button on our form, to force the #ajax that
                // will show our contents. Note that Drupal submit buttons use mousedown
                // and not click
                $(this).parents("form:first").find(".form-submit:first").mousedown();
            });
        });
    }

    Drupal.behaviors.ajaxPagerExample = {
        attach:function()
        {
            // Everytime new content is loaded,
            // any pager should have its links ajaxified.
            pagerListener();
        }
    };
}(jQuery, Drupal));
