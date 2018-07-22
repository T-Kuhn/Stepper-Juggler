using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;

public class FadeOutAndDestroy : MonoBehaviour
{
    private MeshRenderer meshRenderer;

    void Start()
    {
        meshRenderer = GetComponentInChildren<MeshRenderer>();

        meshRenderer.material.DOColor(new Color(0f, 0f, 0f, 0f), "_TintColor", 10f).OnComplete(() => Destroy(this.gameObject));
    }
}
