<?php

/**
 * @file
 * Describes plugin for example block content type.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Example block'),
  'description' => t('Example block.'),
  // 'single' => TRUE means has no subtypes.
  'single' => TRUE,
  // Constructor.
  'content_types' => array('no_context_content_type'),
  // Name of a function which will render the block.
  'render callback' => 'ctools_example_example_block_render',
  // The default context.
  'defaults' => array(),

  // This explicitly declares the config form. Without this line, the func would be
  // ctools_plugin_example_no_context_content_type_edit_form.
  'edit form' => 'ctools_example_example_block_edit_form',
  'icon' => 'favicon.ico',
  'category' => array(t('Example'), -9),
);

/**
 * Run-time rendering of the body of the block.
 *
 * @param $subtype
 * @param $conf
 *   Configuration as done at admin time.
 * @param $args
 * @param $context
 *   Context - in this case we don't have any.
 *
 * @return
 *   An object with at least title and content members.
 */
function ctools_example_example_block_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->content = '';
  if (!empty($conf)) {
    preg_match('/(.*) \[nid:([\d]+)\]/', $conf['title_nid'], $m);
    if (isset($m[1]) && is_numeric($m[2])) {
      $nid = $m[2];
      $node = node_load($nid);
      if ($conf['display_title']) {
        $block->title = $conf['title_as_link'] ? l($node->title, "node/{$node->nid}") : $node->title;
      }
      $lang = field_language('node', $node, 'body');
      if ($conf['show_body']) {
        $block->content .= $node->body[$lang][0]['value'];
      }
    }
  }
  return $block;
}

/**
 * Generates static block edit form.
 */
function ctools_example_example_block_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  // We don't want to use standart pane title functionallity.
  $form['override_title']['#access'] = FALSE;
  $form['override_title_text']['#access'] = FALSE;
  $form['override_title_markup']['#access'] = FALSE;
  $form['title_nid'] = array(
    '#type' => 'textfield',
    '#title' => t('Enter content title'),
    '#autocomplete_path' => 'ctools_example/autocomplete',
    '#default_value' =>  $conf['title_nid'] ? $conf['title_nid'] : '',
  );
  $form['show_body'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show body'),
    '#default_value' =>  $conf['show_body'] ? TRUE : FALSE,
  );
  $form['display_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display title'),
    '#default_value' =>  $conf['display_title'] ? TRUE : FALSE,
  );
  $form['title_as_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display title as link'),
    '#default_value' =>  $conf['title_as_link'] ? TRUE : FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="display_title"]' => array('checked' => TRUE),
      ),
    ),
  );

  return $form;
}

/**
 * Validate handler for example block edit form.
 */
function ctools_example_example_block_edit_form_validate($form, &$form_state) {
  $text = $image = $video = $audio = FALSE;
  if (!preg_match('/.*\[nid:([\d]+)\]/', $form_state['values']['title_nid'], $m)) {
    form_set_error('title_nid', t('Title should be in format "Title [nid:xxx]"'));
  }
}

/**
 * Submit handler for example block edit form.
 */
function ctools_example_example_block_edit_form_submit($form, &$form_state) {
  foreach (array('title_nid', 'display_title', 'title_as_link', 'show_body') as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}
