<?php
/**
 * @file
 * Definition of appnovation_fillout_viewsfield_handler_fillout.
 */

/**
 * Provides a field which will display a link to edit the node if any other listed fields are empty.
 */
class appnovation_fillout_viewsfield_handler_fillout extends views_handler_field_node_link_edit {
  /**
   * Override views_handler_field_node_link_edit::render_link()
   */
  function render_link($node, $values) {
    // Get all field handlers on this display
    $fields = $this->view->display_handler->get_handlers('field');
    // Check if ANY field is empty
    foreach($fields as $key => $field) {
      // ignore our field (duh)
      if ($key !== 'fillout') {
        if (empty($field->get_value($values))) {
          // Let the views hanlder provide the link, it checks for access etc ;)
          return parent::render_link($node, $values);
        }
      }
    }
  }

}
