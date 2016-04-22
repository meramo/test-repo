<?php

$plugin = array(
  'title' => t('Read more link'),
  'description' => t('Displays a read more link.'),
  'single' => TRUE,
  'content_types' => array('read_more'), // machine name
  'render callback' => 'read_more_content_type_render',
  'required context' => new ctools_context_required(t('Node'), 'node'),
  'edit form' => 'read_more_content_type_edit_form',
  'category' => array(t('Morpht Examples'), -9),
);

/**
 * Ctools edit form.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function read_more_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['read_more_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#description' => t('The read more label.'),
    '#default_value' => !empty($conf['read_more_label']) ? $conf['read_more_label'] : 'Read more',
  );
  return $form;
}

/**
 * Ctools edit form submit handler.
 *
 * @param $form
 * @param $form_state
 */
function read_more_content_type_edit_form_submit($form, &$form_state) {
  foreach (array('read_more_label') as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Render callback function.
 *
 * @param $subtype
 * @param $conf
 * @param $args
 * @param $context
 * @return stdClass
 */
function read_more_content_type_render($subtype, $conf, $args, $context) {
  $node = $context->data;
  $block = new stdClass();

  $block->content = l(t("@read_more", array('@read_more' => $conf['read_more_label'])), 'node/' . $node->nid);

  return $block;
}
