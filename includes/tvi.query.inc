<?php
/**
 * TVI Database Interface
 */

/**
 * Return a default setting object.
 */
function tvi_default_settings($type = TVI_TYPE_TERM, $xid = 0) {
  $settings = new stdClass();

  $settings->type = $type;
  $settings->xid  = $xid;

  $settings->view_name = 0;
  $settings->display = TVI_DEFAULT_DISPLAY;
  $settings->status  = 0;
  $settings->inherit = 0;

  $settings->is_default = TRUE;

  return $settings;
}

/**
 * Load a setting from the database or return a default, if the $return_default
 * flag is set.
 */
function tvi_load_settings($xid, $type = TVI_TYPE_TERM, $return_default = TRUE) {
  $xid = _tvi_get_xid($xid, $type);

  $settings = variable_get('tvi_' . $type . '_' . $xid, FALSE);
  $settings = is_array($settings) ? (object) $settings : $settings;

  // Special case where the uuid module has been enabled and where terms 
  // settings are always stored under their tid.
  if (empty($settings) && $type == TVI_TYPE_TERM && module_exists('uuid')) {
    // Get the tid of the term.
    $tid = reset(entity_get_id_by_uuid('taxonomy_term', array($xid)));
    // Check if the xid was not already a tid because uuid was not defined.
    if ($tid != $xid) {
      // Retreive the old settings.
      $settings = variable_get('tvi_' . $type . '_' . $tid, FALSE);
      $settings = is_array($settings) ? (object) $settings : $settings;
      if (!empty($settings)) {
        // Save them using the uuid instead of the tid.
        $settings->xid = $xid;
        tvi_update_settings($settings);
        // Remove the old setting.
        variable_del('tvi_' . $type . '_' . $tid);
      }      
    }
  }

  if (!is_object($settings)) {
    return ($return_default ? tvi_default_settings($type, $xid) : FALSE);
  }

  $settings->is_default = FALSE;
  return $settings;
}

/**
 * Save settings information for a taxonomy vocabulary or term to the database.
 */
function tvi_update_settings($settings) {
  if (is_object($settings)) {
    $settings = (array) $settings;
  }

  variable_set('tvi_' . $settings['type'] . '_' . $settings['xid'], $settings);
}

/**
 * Delete settings information for a taxonomy vocabulary or term in 
 * the database.
 */
function tvi_remove_settings($xid, $type = TVI_TYPE_TERM) {
  $xid = _tvi_get_xid($xid, $type);

  variable_del('tvi_' . $type . '_' . $xid);
}

/**
 * Convert the entity id into its best parameter.
 *
 * Vocabulary's vid converts to machine_name.
 * Term's tid converts to uuid if set.
 */
function _tvi_get_xid($xid, $type = TVI_TYPE_TERM) {
  if ($type == TVI_TYPE_VOCAB) {
    $vocabulary = taxonomy_vocabulary_load($xid);
    if (!empty($vocabulary->machine_name)) {
      $xid = $vocabulary->machine_name;
    }
  }
  elseif ($type == TVI_TYPE_TERM && module_exists('uuid')) {
    $uuid = reset(entity_get_uuid_by_id('taxonomy_term', array($xid)));
    if (!empty($uuid)) {
      $xid = $uuid;
    }
  }
  return $xid;
}
